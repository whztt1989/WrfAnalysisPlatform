#include "wrf_statistic_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "wrf_common.h"

WrfStatisticSolver::WrfStatisticSolver(){
	
}

WrfStatisticSolver::~WrfStatisticSolver(){

}

bool WrfStatisticSolver::GetWindVarianceMap(std::vector< RecordSetMap* >& bias_set_vec, 
	WeightMap& grid_model_weight, std::vector< float >& grid_alpha_map, int grid_size,
	int current_time, WrfGridValueMap* wind_var_map){

	std::vector< std::vector< float > >* model_weight = grid_model_weight.at(current_time - 1);

	wind_var_map->values.resize(wind_var_map->longitude_grid_number * wind_var_map->latitude_grid_number, 0);
	memset(wind_var_map->values.data(), 0, wind_var_map->values.size() * sizeof(float));

	std::vector< float > x_values, y_values;
	float his_x_values, his_y_values;
	float xmean, ymean;
	x_values.resize(bias_set_vec.size());
	y_values.resize(bias_set_vec.size());
	int longi_grid_size = wind_var_map->longitude_grid_number / grid_size;
	int lati_grid_size = wind_var_map->latitude_grid_number / grid_size;
	for ( int lati = 0; lati < wind_var_map->latitude_grid_number; ++lati){
		int temp_lati_index = lati / grid_size;
		if ( temp_lati_index >= lati_grid_size ) continue;
		for ( int longi = 0; longi < wind_var_map->longitude_grid_number; ++longi){
			int temp_longi_index = longi / grid_size;
			if ( temp_longi_index >= longi_grid_size ) continue;

			int temp_grid_index = temp_lati_index * longi_grid_size + temp_longi_index;
			int temp_index = lati * wind_var_map->longitude_grid_number + longi;
			his_x_values = 0;
			his_y_values = 0;
			for ( int k = 0; k < bias_set_vec.size(); ++k ){
				WrfDataRecordSet* data_set = bias_set_vec[k]->at(current_time - 1);
				float temp_weight = model_weight->at(temp_grid_index)[k];
				x_values[k] = data_set->values[temp_index]->x_speed_850hpa * temp_weight;
				y_values[k] = data_set->values[temp_index]->y_speed_850hpa * temp_weight;
				his_x_values += data_set->his_values[temp_index]->x_speed_850hpa * temp_weight;
				his_y_values += data_set->his_values[temp_index]->y_speed_850hpa * temp_weight;
			}
			xmean = 0;
			ymean = 0;
			for ( int k = 0; k < bias_set_vec.size(); ++k ){
				xmean += x_values[k];
				ymean += y_values[k];
			}
			xmean /= bias_set_vec.size();
			ymean /= bias_set_vec.size();
			float variancex = 0, variancey = 0;
			for ( int k = 0; k < bias_set_vec.size(); ++k ){
				variancex += pow((x_values[k] - xmean), 2);
				variancey += pow((y_values[k] - ymean), 2);
			}
			variancex *= grid_alpha_map[temp_index];
			variancex *= grid_alpha_map[temp_index];
			wind_var_map->values[temp_index] = variancex / abs(his_x_values) + variancey / abs(his_y_values);
		}
	}

	float max_variance;
	float min_variance;
	SortValues(wind_var_map->values, max_variance, min_variance);

	for ( int i = 0; i < wind_var_map->values.size(); ++i ) {
		wind_var_map->values[i] = (wind_var_map->values[i] - min_variance) / (max_variance - min_variance);
		if ( wind_var_map->values[i] > 1 ) wind_var_map->values[i] = 1;
		if ( wind_var_map->values[i] < 0 ) wind_var_map->values[i] = 0;
	}

	return true;
}

bool WrfStatisticSolver::GetSimilarityMap(int longitude_index, int latitude_index, int current_time, int time_length,
	std::vector< RecordSetMap* >& bias_set_vec, DataStampMap& historical_map, 
	std::vector< float >& attrib_weight, std::vector< float >& model_weight, std::vector< float >& attrib_max_values, std::vector< float >& attrib_min_values,
	WrfGridValueMap* similarity_map){
	
	float max_value = -1e10;
	float min_value = 1e10;
	int selected_index = latitude_index * similarity_map->longitude_grid_number + longitude_index;
	for ( int i = 0 ; i < similarity_map->latitude_grid_number; ++i )
		for ( int j = 0; j < similarity_map->longitude_grid_number; ++j ){
			float acc_value = 0;
			float dis_scale = sqrt(pow((float)(longitude_index - j) / similarity_map->longitude_grid_number, 2) + pow((float)(latitude_index - i) / similarity_map->latitude_grid_number, 2));
			int current_index = i * similarity_map->longitude_grid_number + j;
			if ( selected_index == current_index ) continue;
			for ( int length = 0; length < time_length; ++length ){
				float time_acc_value = 0;
				int temp_time = current_time - length;
				for ( int k = 0; k < bias_set_vec.size(); ++k ){
					float model_acc_value = 0;
					RecordSetMap* record_set_map = bias_set_vec.at(k);
					WrfDataRecordSet* bias_data_record = record_set_map->at(temp_time);
					std::vector< WrfDataStamp* >* his_data_vec = historical_map.at(temp_time);
					for ( int n = 0; n < his_data_vec->size() - 1; ++n ){
						WrfGridDataStamp* grid_stamp = dynamic_cast< WrfGridDataStamp* >(his_data_vec->at(n));
						float temp_value = 0.0;
						switch ( grid_stamp->data_type() ){
						case WRF_RAIN:
							temp_value = abs(bias_data_record->values[selected_index]->rain - bias_data_record->values[current_index]->rain);
							break;
						case WRF_PRESSURE:
							temp_value = abs(bias_data_record->values[selected_index]->pressure - bias_data_record->values[current_index]->pressure);
							break;
						case WRF_TEMPERATURE_850HPA:
							temp_value = abs(bias_data_record->values[selected_index]->temperature_850hpa - bias_data_record->values[current_index]->temperature_850hpa);
							break;
						case WRF_WIND_850HPA:
							temp_value = abs(bias_data_record->values[selected_index]->x_speed_850hpa - bias_data_record->values[current_index]->x_speed_850hpa);
							temp_value += abs(bias_data_record->values[selected_index]->y_speed_850hpa - bias_data_record->values[current_index]->y_speed_850hpa);
							break;
						case WRF_RH_850HPA:
							//temp_value = abs(bias_data_record->values[selected_index]->relative_humidity_850hpa - bias_data_record->values[current_index]->relative_humidity_850hpa);
							break;
						case WRF_HEIGHT_850HPA:
							temp_value = abs(bias_data_record->values[selected_index]->height_850hpa - bias_data_record->values[current_index]->height_850hpa);
							break;
						default:
							break;
						}
						model_acc_value +=  temp_value * abs(grid_stamp->values[selected_index] - grid_stamp->values[current_index]) * attrib_weight[grid_stamp->data_type()];
					}
					time_acc_value += model_acc_value * model_weight[k] * dis_scale;
				}
				acc_value += time_acc_value * Ftime(length);
			}
			//similarity_map->values[current_index] = -1.0 * log(acc_value);
			//similarity_map->values[current_index] = 1.0 / acc_value;
			similarity_map->values[current_index] = 1.0 / dis_scale;
			if ( similarity_map->values[current_index] > max_value ) max_value = similarity_map->values[current_index];
			if ( similarity_map->values[current_index] < min_value ) min_value = similarity_map->values[current_index];
		}
	for ( int i = 0; i < similarity_map->values.size(); ++i )
		similarity_map->values[i] = (similarity_map->values[i] - min_value) / (max_value - min_value);
	similarity_map->values[selected_index] = 1.0;
	return false;
}

bool WrfStatisticSolver::GetBiasVarMap(std::vector< RecordSetMap* >& bias_set_vec,
	std::vector< float >& attrib_weight, WeightMap& grid_model_weight, int grid_size,
	std::vector< float >& grid_alpha_map, int current_time, int his_time_length, float history_weight, float current_weight, 
	WrfGridValueMap* bias_var_map){
	
	int longi_grid_size = bias_var_map->longitude_grid_number / grid_size;
	int lati_grid_size = bias_var_map->latitude_grid_number / grid_size;
	memset(bias_var_map->values.data(), 0, bias_var_map->values.size() * sizeof(float));

	for ( int i = 0; i < bias_var_map->latitude_grid_number; ++i ){
		int temp_lati_index = i / grid_size;
		if ( temp_lati_index >= lati_grid_size ) continue;
		for ( int j = 0; j < bias_var_map->longitude_grid_number; ++j ){
			int temp_longi_index = j / grid_size;
			if ( temp_longi_index >= longi_grid_size ) continue;

			int temp_grid_index = temp_lati_index * longi_grid_size + temp_longi_index;
			int grid_index = i * bias_var_map->longitude_grid_number + j;

			WrfDataRecord bias_record;
			float result_value = 0;
			for ( int k = 0; k < bias_set_vec.size(); ++k ){
				float temp_result_value = 0;
				float temp_weight = 0;
				for ( int t = current_time - his_time_length; t < current_time; ++t ){
					std::vector< std::vector< float > >* model_weight = grid_model_weight.at(t);

					WrfDataRecordSet* record_set = bias_set_vec[k]->at(t);
					WrfDataRecord* temp_record = record_set->values[grid_index];
					WrfDataRecord* his_record = record_set->his_values[grid_index];

					bias_record.rain += abs(temp_record->rain) / (abs(his_record->rain) + 0.1 + 5) * attrib_weight[WRF_RAIN];
					bias_record.pressure += abs(temp_record->pressure) / (abs(his_record->pressure) + 0.1 + 2) * attrib_weight[WRF_PRESSURE];
					bias_record.relative_humidity_850hpa += abs(temp_record->relative_humidity_850hpa) / (abs(his_record->relative_humidity_850hpa) + 0.1 + 10) * attrib_weight[WRF_RH_850HPA];
					bias_record.height_850hpa += abs(temp_record->height_850hpa) / (abs(his_record->height_850hpa) + 0.1 + 2) * attrib_weight[WRF_HEIGHT_850HPA];
					bias_record.temperature_850hpa += abs(temp_record->temperature_850hpa) / (abs(his_record->temperature_850hpa) + 0.1 + 2) * attrib_weight[WRF_TEMPERATURE_850HPA];
					bias_record.x_speed_850hpa += abs(temp_record->x_speed_850hpa) / (abs(his_record->x_speed_850hpa) + 0.1 + 10) * attrib_weight[WRF_WIND_X_850HPA];
					bias_record.y_speed_850hpa += abs(temp_record->y_speed_850hpa) / (abs(his_record->y_speed_850hpa) + 0.1 + 10) * attrib_weight[WRF_WIND_Y_850HPA];

					float temp_scale = Ftime(current_time - t);
					temp_weight += temp_scale;
					bias_record *= temp_scale;
					bias_record *= model_weight->at(temp_grid_index)[k];

					temp_result_value += bias_record.ToSum();
				}
				if ( temp_weight > 1e-5 ) temp_result_value /= temp_weight;

				WrfDataRecordSet* current_set = bias_set_vec[k]->at(current_time);
				WrfDataRecordSet* prev_set = bias_set_vec[k]->at(current_time - 1);
				WrfDataRecord* current_record = current_set->values[grid_index];
				WrfDataRecord* pre_record = prev_set->values[grid_index];
				WrfDataRecord* pre_his_record = prev_set->his_values[grid_index];
				float scale = 0;
				scale += abs(current_record->rain - pre_record->rain - pre_his_record->rain) / (abs(pre_record->rain + pre_his_record->rain) + 5 + 0.1) * attrib_weight[WRF_RAIN];
				scale += abs(current_record->pressure - pre_record->pressure - pre_his_record->pressure) / (abs(pre_record->pressure + pre_his_record->pressure) + 2 + 0.1) * attrib_weight[WRF_PRESSURE];
				scale += abs(current_record->relative_humidity_850hpa - pre_record->relative_humidity_850hpa - pre_his_record->relative_humidity_850hpa) / (abs(pre_record->relative_humidity_850hpa + pre_his_record->relative_humidity_850hpa) + 10 + 0.1) * attrib_weight[WRF_RH_850HPA];
				scale += abs(current_record->height_850hpa - pre_record->height_850hpa - pre_his_record->height_850hpa) / (abs(pre_record->height_850hpa + pre_his_record->height_850hpa) + 2 + 0.1) * attrib_weight[WRF_HEIGHT_850HPA];
				scale += abs(current_record->temperature_850hpa - pre_record->temperature_850hpa - pre_his_record->temperature_850hpa) / (abs(pre_record->temperature_850hpa + pre_his_record->temperature_850hpa) + 2 + 0.1) * attrib_weight[WRF_TEMPERATURE_850HPA];
				scale += abs(current_record->x_speed_850hpa - pre_record->x_speed_850hpa - pre_his_record->x_speed_850hpa) / (abs(pre_record->x_speed_850hpa + pre_his_record->x_speed_850hpa) + 2 + 0.1) * attrib_weight[WRF_WIND_X_850HPA];
				scale += abs(current_record->y_speed_850hpa - pre_record->y_speed_850hpa - pre_his_record->y_speed_850hpa) / (abs(pre_record->y_speed_850hpa + pre_his_record->y_speed_850hpa) + 2 + 0.1) * attrib_weight[WRF_WIND_Y_850HPA];
				result_value += temp_result_value * history_weight + scale * current_weight;
			}
			result_value /= bias_set_vec.size();
			bias_var_map->values[grid_index] = result_value;
		}
	}
	
	float max_sum = -1, min_sum = 1e20;
	SortValues(bias_var_map->values, max_sum, min_sum);
	
	for ( int i = 0; i < bias_var_map->latitude_grid_number * bias_var_map->longitude_grid_number; ++i ){
		bias_var_map->values[i] = (bias_var_map->values[i] - min_sum) / (max_sum - min_sum);
		if ( bias_var_map->values[i] > 1 ) bias_var_map->values[i] = 1;
		if ( bias_var_map->values[i] < 0 ) bias_var_map->values[i] = 0;
	}

	bias_var_map->level = 0;
	bias_var_map->weight.resize(attrib_weight.size());
	bias_var_map->weight.assign(attrib_weight.begin(), attrib_weight.end());

	return true;
}

float WrfStatisticSolver::GetHittingRate(std::vector< RecordSetMap* >& bias_set_vec, 
	std::vector< float >& attrib_weight, WeightMap& grid_model_weight, int grid_size, 
	std::vector< float >& grid_alpha_map, int current_time, int his_time_length, float threshold, 
	WrfGridValueMap* test_map){

	std::vector< float > hitting_count, temp_hitting_count;
	float bias_hitting_count = 0;
	hitting_count.resize(attrib_weight.size(), 0);
	temp_hitting_count.resize(attrib_weight.size(), 0);

	//GetBiasVarMap(bias_set_vec, attrib_weight, grid_model_weight, grid_size, grid_alpha_map, current_time, his_time_length, test_map);

	int longi_grid_size = test_map->longitude_grid_number / grid_size;
	int lati_grid_size = test_map->latitude_grid_number / grid_size;

	for ( int i = 0; i < test_map->latitude_grid_number; ++i ){
		int temp_lati_index = i / grid_size;
		if ( temp_lati_index >= lati_grid_size ) continue;
		for ( int j = 0; j < test_map->longitude_grid_number; ++j ){
			int temp_longi_index = j / grid_size;
			if ( temp_longi_index >= longi_grid_size ) continue;

			int temp_grid_index = temp_lati_index * longi_grid_size + temp_longi_index;
			int grid_index = i * test_map->longitude_grid_number + j;
			
			if ( test_map->values[grid_index] > threshold ) continue;

			bias_hitting_count += 1;
			for ( int k = 0; k < bias_set_vec.size(); ++k ){
				for ( int t = current_time - his_time_length; t < current_time; ++t ){
					std::vector< std::vector< float > >* model_weight = grid_model_weight.at(t);
					float temp_time_value = Ftime(current_time - t);

					WrfDataRecordSet* record_set = bias_set_vec[k]->at(t);
					WrfDataRecord* temp_record = record_set->values[grid_index];
					WrfDataRecord* his_record = record_set->his_values[grid_index];
					if ( abs(temp_record->rain) / (abs(his_record->rain) + 0.1) < threshold ) hitting_count[WRF_RAIN] += temp_time_value;
					if ( abs(temp_record->pressure) / (abs(his_record->pressure) + 0.1) < threshold ) hitting_count[WRF_PRESSURE] += temp_time_value;
					if ( abs(temp_record->relative_humidity_850hpa) / (abs(his_record->relative_humidity_850hpa) + 0.1) < threshold ) hitting_count[WRF_RH_850HPA] += temp_time_value;
					if ( abs(temp_record->height_850hpa) / (abs(his_record->height_850hpa) + 0.1) < threshold ) hitting_count[WRF_HEIGHT_850HPA] += temp_time_value;
					if ( abs(temp_record->temperature_850hpa) / (abs(his_record->temperature_850hpa) + 0.1) < threshold ) hitting_count[WRF_TEMPERATURE_850HPA] += temp_time_value;
					if ( abs(temp_record->x_speed_850hpa) / (abs(his_record->x_speed_850hpa) + 0.1) < threshold ) hitting_count[WRF_WIND_X_850HPA] += temp_time_value;
					if ( abs(temp_record->y_speed_850hpa) / (abs(his_record->y_speed_850hpa) + 0.1) < threshold ) hitting_count[WRF_WIND_Y_850HPA] += temp_time_value;
				}
			}
		}
	}

	float time_acc = 0;
	for ( int t = current_time - his_time_length; t < current_time; ++t ) time_acc += Ftime(current_time - t);
	//float node_num = longi_grid_size * lati_grid_size * grid_size * grid_size * bias_set_vec.size() * time_acc;
	bias_hitting_count *= time_acc * bias_set_vec.size();
	for ( int i = 0; i < 7; ++i ) hitting_count[i] /= bias_hitting_count;
	float theropy = 0;
	for ( int i = 0; i < 7; ++i ) 
		if (hitting_count[i] != 0 ) theropy += -1 * hitting_count[i] * log(hitting_count[i]);

	return theropy;
}

float WrfStatisticSolver::GetMapDifference(WrfGridValueMap* src, WrfGridValueMap* dst){
	double bias_variance = 0;
	for (int i = 0; i < src->values.size(); ++i ){
		bias_variance += abs(src->values[i] - dst->values[i]) / (abs(src->values[i]) + abs(dst->values[i]) + 0.1);
	}
	bias_variance /= src->values.size();

	return bias_variance;
}

float WrfStatisticSolver::GetMutalDistance(WrfGridValueMap* src, WrfGridValueMap* dst){
	int segment_num = 4;
	float step = 1.0 / segment_num;

	std::vector< std::vector< float > > pxy;
	std::vector< float > px;
	std::vector< float > py;

	pxy.resize(segment_num);
	for ( int i = 0; i < segment_num; ++i ) {
		pxy[i].resize(segment_num);
		pxy[i].assign(pxy[i].size(), 0);
	}
	px.resize(segment_num);
	px.assign(px.size(), 0);
	py.resize(segment_num);
	py.assign(py.size(), 0);

	int x_index, y_index;
	for ( int i = 0; i < src->values.size(); ++i ){
		x_index = (int)(src->values[i] / step);
		y_index = (int)(dst->values[i] / step);
		if ( x_index >= segment_num ) x_index = segment_num - 1;
		if ( y_index >= segment_num ) y_index = segment_num - 1;

		pxy[x_index][y_index] += 1;
		px[x_index] += 1;
		py[y_index] += 1;
	}

	for ( int i = 0; i < segment_num; ++i )
		for ( int j = 0; j < segment_num; ++j )
			pxy[i][j] /= src->values.size();
	for ( int i = 0; i < segment_num; ++i ){
		px[i] /= src->values.size();
		py[i] /= src->values.size();
	}

	float mutal_distance = 0;
	for ( int i = 0; i < segment_num; ++i ){
		for ( int j = 0; j < segment_num; ++j ){
			if ( px[i] < 1e-10 || py[i] < 1e-10 || pxy[i][j] < 1e-10 ) continue;
			mutal_distance += pxy[i][j] * log(pxy[i][j] / (px[i] * py[j]));
		}
	}
	return mutal_distance;
}

float WrfStatisticSolver::Ftime(int time_bias){
	//return 1.0 / abs(time_bias + 0.1);
	return exp(-1.0 * abs(time_bias));
}

void WrfStatisticSolver::SortValues(std::vector< float >& values, float& max_value, float& min_value){
	std::vector< float > temp_values;
	temp_values.resize(values.size());
	temp_values.assign(values.begin(), values.end());

	QuickSort(0, temp_values.size() - 1, temp_values.data());

	int max_index = (int)(0.95 * temp_values.size());
	int min_index = (int)(0.05 * temp_values.size());
	max_value = temp_values[max_index];
	min_value = temp_values[min_index];
}

void WrfStatisticSolver::QuickSort(int head, int end, float* data){
	int h = head;
	int e = end;

	float temp = data[h];
	while ( h < e ){
		while ( data[e] >= temp && e > h ) e--;
		data[h] = data[e];
		while ( data[h] <= temp && e > h ) h++;
		data[e] = data[h];
	}
	data[h] = temp;

	if ( h - 1 > head ) QuickSort(head, h - 1, data);
	if ( h + 1 < end) QuickSort(h + 1, end, data);
}

void WrfStatisticSolver::TestSort(){
	std::vector< float > values;
	values.resize(100);
	srand((unsigned int)time(0));
	for ( int i = 0; i < 100; ++i ) values[i] = (float)rand() / RAND_MAX;
	float max_value, min_value;
	SortValues(values, max_value, min_value);
}