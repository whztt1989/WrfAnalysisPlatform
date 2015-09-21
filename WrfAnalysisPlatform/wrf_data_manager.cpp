#include "wrf_data_manager.h"

#include <iostream>
#include <QStringList>
#include <QDir>

#include "wrf_statistic_solver.h"
#include "lapacke.h"
#include "system_timer.h"

WrfDataManager* WrfDataManager::instance_ = 0;

WrfDataManager* WrfDataManager::GetInstance(){
	if ( instance_ == 0 ){
		instance_ = new WrfDataManager;
	}
	return instance_;
}

bool WrfDataManager::DeleteInstance(){
	if ( instance_ != 0 ){
		delete instance_;
		instance_ = 0;
		return true;
	}
	return false;
}

WrfDataManager::WrfDataManager(){
	/*start_longitude_ = 60.0f;
	end_longitude_ = 179.250f;
	start_latitude_ = 0.0f;
	end_latitude_ = 59.625f;
	longitude_grid_space_ = 1.125f;
	latitude_grid_space_ = 1.125f;
	longitude_grid_number_ = 107;
	latitude_grid_number_ = 54;*/
	start_longitude_ = 60.0f;
	end_longitude_ = 150.0f;
	start_latitude_ = 15.0f;
	end_latitude_ = 60.0f;
	/*longitude_grid_space_ = 1.125f;
	latitude_grid_space_ = 1.125f;
	longitude_grid_number_ = 81;
	latitude_grid_number_ = 41;*/

	longitude_grid_space_ = 0.25f;
	latitude_grid_space_ = 0.25f;
	longitude_grid_number_ = 361;
	latitude_grid_number_ = 181;

	site_alpha_.resize(longitude_grid_number_ * latitude_grid_number_, 0);

	grid_size_ = 3;
	history_weight_ = 0.5;
	current_weight_ = 0.5;
	
	//attribute_weight_.resize(7, 0.143);
	/*attribute_weight_.push_back(1.0);
	attribute_weight_.push_back(0.1);
	attribute_weight_.push_back(0.1);
	attribute_weight_.push_back(0.05);
	attribute_weight_.push_back(0.05);
	attribute_weight_.push_back(0.1);
	attribute_weight_.push_back(0.1);*/
	attribute_weight_.resize(7, 0);
	attribute_weight_[0] = 1.0;

	model_weight_.resize(4, 0.25);

	attribute_max_value_.resize(10, -1e10);
	attribute_min_value_.resize(10, 1e10);

	similarity_threshold_ = 0.7;
	wind_var_threshold_ = 0.5;

	data_converter_ = new WrfDataConverter;
	data_converter_->set_start_latitude(start_latitude_);
	data_converter_->set_start_longitude(start_longitude_);
	data_converter_->set_end_latitude(end_latitude_);
	data_converter_->set_end_longitude(end_longitude_);
	data_converter_->set_longitude_grid_space(longitude_grid_space_);
	data_converter_->set_latitude_grid_space(latitude_grid_space_);
	data_converter_->set_longtitude_grid_number(longitude_grid_number_);
	data_converter_->set_latitude_grid_number(latitude_grid_number_);

	statistic_solver_ = new WrfStatisticSolver;
	statistic_solver_->TestSort();
	bias_map_ = NULL;

	bias_parallel_dataset_ = NULL;
	absolute_parallel_dataset_ = NULL;
	bias_line_chart_dataset_ = NULL;
	absolute_line_chart_dataset_ = NULL;
	compared_bias_parallel_dataset_ = NULL;
	compared_absolute_parallel_dataset_ = NULL;
	compared_bias_line_chart_dataset_ = NULL;
	compared_absolute_line_chart_dataset_ = NULL;

	histogram_dataset_ = NULL;
	wind_var_map_ = NULL;

	current_time_ = WrfTime(13, 04, 20, 8).ToInt();
	history_time_length_ = 3;
}

WrfDataManager::~WrfDataManager(){

}

void WrfDataManager::GetSiteAlpha(std::vector< float >& alpha){
	alpha.resize(site_alpha_.size());
	alpha.assign(site_alpha_.begin(), site_alpha_.end());
}

void WrfDataManager::SetHighLightIndex(std::vector< bool >& index){
	high_light_index_.resize(index.size());
	high_light_index_.assign(index.begin(), index.end());
}

void WrfDataManager::GetHighLightIndex(std::vector< bool >& index){
	index.resize(high_light_index_.size());
	index.assign(high_light_index_.begin(), high_light_index_.end());
}

void WrfDataManager::GetAttribWeight(std::vector< float >& weight){
	weight.resize(attribute_weight_.size());
	weight.assign(attribute_weight_.begin(), attribute_weight_.end());
}

void WrfDataManager::SetAttribWeight(std::vector< float >& weight){
	if ( weight.size() != attribute_weight_.size() ) return;
	attribute_weight_.assign(weight.begin(), weight.end());
}

void WrfDataManager::GetModelWeight(std::vector< float >& weight){
	weight.resize(model_weight_.size());
	weight.assign(model_weight_.begin(), model_weight_.end());
}

void WrfDataManager::GetAttribWeightTheropy(int step_num,  std::vector< std::vector< float > >& theropy){
	std::vector< float > temp_attrib_weight;
	temp_attrib_weight.resize(attribute_weight_.size());

	WrfGridValueMap* test_map = new WrfGridValueMap;
	test_map->start_latitude = start_latitude_;
	test_map->end_latitude = end_latitude_;
	test_map->start_longitude = start_longitude_;
	test_map->end_longitude = end_longitude_;
	test_map->latitude_grid_number = latitude_grid_number_;
	test_map->longitude_grid_number = longitude_grid_number_;
	test_map->latitude_grid_space = latitude_grid_space_;
	test_map->longitude_grid_space = longitude_grid_space_;
	test_map->values.resize(test_map->longitude_grid_number * test_map->latitude_grid_number);
	memset(test_map->values.data(), 0, test_map->values.size() * sizeof(float));

	float max_value = -1e10;
	float min_value = 1e10;
	theropy.resize(attribute_weight_.size());
	float step_size = 1.0 / step_num;
	for ( int i = 0; i < attribute_weight_.size(); ++i ){
		theropy[i].resize(step_num);
		temp_attrib_weight.assign(attribute_weight_.begin(), attribute_weight_.end());
		for ( int k = 0; k < step_num; ++k ){
			float temp_weight = step_size * k;
			float average = (temp_attrib_weight[i] - temp_weight) /  (temp_attrib_weight.size() - 1);
			temp_attrib_weight[i] = temp_weight;
			for ( int j = 0; j < i; ++j ) if ( j != i ) temp_attrib_weight[j]  += average;
			theropy[i][k] = statistic_solver_->GetHittingRate(bias_set_vec_, temp_attrib_weight, grid_model_weight_map_, grid_size_, site_alpha_, current_time_, history_time_length_, 0.3, test_map);
			if ( theropy[i][k] < min_value ) min_value = theropy[i][k];
			if ( theropy[i][k] > max_value ) max_value = theropy[i][k];
		}	
	}
	for ( int i = 0; i < attribute_weight_.size(); ++i )
		for ( int k = 0; k < step_num; ++k )
			theropy[i][k] = (theropy[i][k] - min_value) / (max_value - min_value);

	delete test_map;
}

WrfGridValueMap* WrfDataManager::GetBiasMap(){
	if ( bias_map_ == 0 ) bias_map_ = new WrfGridValueMap();

	bias_map_->start_latitude = start_latitude_;
	bias_map_->end_latitude = end_latitude_;
	bias_map_->start_longitude = start_longitude_;
	bias_map_->end_longitude = end_longitude_;
	bias_map_->latitude_grid_number = latitude_grid_number_;
	bias_map_->longitude_grid_number = longitude_grid_number_;
	bias_map_->latitude_grid_space = latitude_grid_space_;
	bias_map_->longitude_grid_space = longitude_grid_space_;
	bias_map_->values.resize(bias_map_->longitude_grid_number * bias_map_->latitude_grid_number);
	memset(bias_map_->values.data(), 0, bias_map_->values.size() * sizeof(float));

	SystemTimer::GetInstance()->BeginTimer();
	statistic_solver_->GetBiasVarMap(bias_set_vec_, attribute_weight_, grid_model_weight_map_, grid_size_, site_alpha_, current_time_, history_time_length_, history_weight_, current_weight_, bias_map_);
	SystemTimer::GetInstance()->EndTimer();
	std::cout << "Time used to get bias var map:  " ;
	SystemTimer::GetInstance()->PrintTime();
	std::cout << std::endl;
	//statistic_solver_->GetWindVarianceMap(bias_set_vec_, grid_model_weight_map_, site_alpha_, grid_size_, current_time_, bias_map_);
	bias_map_->max_value = 1.0;
	bias_map_->min_value = 0.0;

	// equalization

	/*int ori_bin[256], accu_bin[256], mapping_bin[256];
	memset(ori_bin, 0, 256 * sizeof(int));
	memset(accu_bin, 0, 256 * sizeof(int));
	for ( int i = 0; i < bias_map_->values.size(); ++i ) ori_bin[(int)(bias_map_->values[i] * 255 + 0.5)]++;
	accu_bin[0] = ori_bin[0];
	for ( int i = 1; i < 256; ++i ) accu_bin[i] = accu_bin[i - 1] + ori_bin[i];
	for ( int i = 0; i < 256; ++i )
		mapping_bin[i] = (int)(255 * (float)accu_bin[i] / bias_map_->values.size() + 0.5);
	for ( int i = 0; i < bias_map_->values.size(); ++i ){
		float mapping_value = mapping_bin[(int)(bias_map_->values[i] * 255 + 0.5)] / 255.0;
		bias_map_->values[i] = mapping_value;
	}*/

	return bias_map_;
}

WrfGridValueMap* WrfDataManager::GetWindVarMap(){
	if ( wind_var_map_ == 0 ) wind_var_map_ = new WrfGridValueMap();

	wind_var_map_->start_latitude = start_latitude_;
	wind_var_map_->end_latitude = end_latitude_;
	wind_var_map_->start_longitude = start_longitude_;
	wind_var_map_->end_longitude = end_longitude_;
	wind_var_map_->latitude_grid_number = latitude_grid_number_;
	wind_var_map_->longitude_grid_number = longitude_grid_number_;
	wind_var_map_->latitude_grid_space = latitude_grid_space_;
	wind_var_map_->longitude_grid_space = longitude_grid_space_;
	wind_var_map_->values.resize(bias_map_->longitude_grid_number * bias_map_->latitude_grid_number);
	memset(wind_var_map_->values.data(), 0, wind_var_map_->values.size() * sizeof(float));

	statistic_solver_->GetWindVarianceMap(bias_set_vec_, grid_model_weight_map_, site_alpha_, grid_size_, current_time_, wind_var_map_);
	wind_var_map_->max_value = 1.0;
	wind_var_map_->min_value = 0.0;

	// equalization

	/*int ori_bin[256], accu_bin[256], mapping_bin[256];
	memset(ori_bin, 0, 256 * sizeof(int));
	memset(accu_bin, 0, 256 * sizeof(int));
	for ( int i = 0; i < wind_var_map_->values.size(); ++i ) ori_bin[(int)(wind_var_map_->values[i] * 255 + 0.5)]++;
	accu_bin[0] = ori_bin[0];
	for ( int i = 1; i < 256; ++i ) accu_bin[i] = accu_bin[i - 1] + ori_bin[i];
	for ( int i = 0; i < 256; ++i )
		mapping_bin[i] = (int)(255 * (float)accu_bin[i] / wind_var_map_->values.size() + 0.5);
	for ( int i = 0; i < bias_map_->values.size(); ++i ){
		float mapping_value = mapping_bin[(int)(wind_var_map_->values[i] * 255 + 0.5)] / 255.0;
		wind_var_map_->values[i] = mapping_value;
	}*/

	return wind_var_map_;
}

WrfParallelDataSet* WrfDataManager::GetParallelDataSet(bool is_absolute){
	WrfParallelDataSet* parallel_dataset;
	if ( !is_absolute )
		parallel_dataset = bias_parallel_dataset_;
	else
		parallel_dataset = absolute_parallel_dataset_;

	if ( parallel_dataset != NULL ) delete parallel_dataset;
	parallel_dataset = new WrfParallelDataSet;

	GetParallelDataSet(selected_grid_index_, is_absolute, parallel_dataset);

	if ( !is_absolute )
		bias_parallel_dataset_ = parallel_dataset;
	else
		absolute_parallel_dataset_ = parallel_dataset;

	return parallel_dataset;
}

WrfParallelDataSet* WrfDataManager::GetComparedParallelDataSet(bool is_absolute){
	WrfParallelDataSet* parallel_dataset;
	if ( !is_absolute )
		parallel_dataset = compared_bias_parallel_dataset_;
	else
		parallel_dataset = compared_absolute_parallel_dataset_;

	if ( parallel_dataset != NULL ) delete parallel_dataset;
	parallel_dataset = new WrfParallelDataSet;

	GetParallelDataSet(compared_grid_index_, is_absolute, parallel_dataset);

	if ( !is_absolute )
		compared_bias_parallel_dataset_ = parallel_dataset;
	else
		compared_absolute_parallel_dataset_ = parallel_dataset;

	return parallel_dataset;
}

void WrfDataManager::GetParallelDataSet(std::vector< bool >& selected_index, bool is_absolute, WrfParallelDataSet* parallel_dataset){
	/*WrfParallelModel model_t639;
	model_t639.model_name = "T639";
	model_t639.r = 1.0;
	model_t639.g = 0.0;
	model_t639.b = 0.0;
	parallel_dataset->model_vec.push_back(model_t639);*/

	WrfParallelModel model_ecfine;
	model_ecfine.model_name = "EcFine";
	model_ecfine.r = 0.0;
	model_ecfine.g = 1.0;
	model_ecfine.b = 0.0;
	parallel_dataset->model_vec.push_back(model_ecfine);

	WrfParallelModel model_ncep;
	model_ncep.model_name = "NCEP";
	model_ncep.r = 0.0;
	model_ncep.g = 0.0;
	model_ncep.b = 1.0;
	parallel_dataset->model_vec.push_back(model_ncep);

	WrfParallelModel model_japangsm;
	model_japangsm.model_name = "Japan_Gsm";
	model_japangsm.r = 0.0;
	model_japangsm.g = 1.0;
	model_japangsm.b = 1.0;
	parallel_dataset->model_vec.push_back(model_japangsm);

	// rain, pressure, temperature, wind_speed, wind_dir, height, rh
	parallel_dataset->attrib_name_vec.resize(7);
	parallel_dataset->attrib_name_vec[WRF_RAIN] = "Rain24";
	parallel_dataset->attrib_name_vec[WRF_PRESSURE] = "Pressure";
	parallel_dataset->attrib_name_vec[WRF_TEMPERATURE_850HPA] = "Temperature_850hpa";
	parallel_dataset->attrib_name_vec[WRF_HEIGHT_850HPA] = "Height_500hpa";
	parallel_dataset->attrib_name_vec[WRF_WIND_X_850HPA] = "Wind_Speed_850hpa";
	parallel_dataset->attrib_name_vec[WRF_WIND_Y_850HPA] = "Wind_Angle_850hpa";
	parallel_dataset->attrib_name_vec[WRF_RH_850HPA] = "Relative Humidity";

	parallel_dataset->min_value_vec.resize(attribute_min_value_.size(), 1e10);
	parallel_dataset->max_value_vec.resize(attribute_max_value_.size(), -1e10);

	parallel_dataset->mapped_axis.resize(parallel_dataset->attrib_name_vec.size());
	for ( int i = 0; i < parallel_dataset->mapped_axis.size(); ++i ) parallel_dataset->mapped_axis[i] = i;

	parallel_dataset->values.resize(parallel_dataset->model_vec.size());

	int longi_grid_size = longitude_grid_number_ / grid_size_;
	int lati_grid_size = latitude_grid_number_ / grid_size_;
	for ( int i = 0; i < longitude_grid_number_ * latitude_grid_number_; ++i ){
		int temp_longi = i % longitude_grid_number_;
		temp_longi /= grid_size_;
		int temp_lati = i / longitude_grid_number_;
		temp_lati /= grid_size_;
		if ( temp_longi < longi_grid_size && temp_lati < lati_grid_size && selected_index[i] ){
			WrfParallelRecord record;
			for ( int k = 0; k < bias_set_vec_.size(); ++k ){
				RecordSetMap* set_map = bias_set_vec_.at(k);

				if ( !is_absolute ){
					for ( int t = current_time_ - history_time_length_; t < current_time_; ++t ){
						WrfDataRecordSet* record_set = set_map->at(t);

						WrfDataRecord* data_record = record_set->values[i];
						record.data[WRF_RAIN] = data_record->rain;
						if ( record.data[WRF_RAIN] > parallel_dataset->max_value_vec[WRF_RAIN] ) parallel_dataset->max_value_vec[WRF_RAIN] = record.data[WRF_RAIN];
						if ( record.data[WRF_RAIN] < parallel_dataset->min_value_vec[WRF_RAIN] ) parallel_dataset->min_value_vec[WRF_RAIN] = record.data[WRF_RAIN];
						record.data[WRF_PRESSURE] = data_record->pressure;
						if ( record.data[WRF_PRESSURE] > parallel_dataset->max_value_vec[WRF_PRESSURE] ) parallel_dataset->max_value_vec[WRF_PRESSURE] = record.data[WRF_PRESSURE];
						if ( record.data[WRF_PRESSURE] < parallel_dataset->min_value_vec[WRF_PRESSURE] ) parallel_dataset->min_value_vec[WRF_PRESSURE] = record.data[WRF_PRESSURE];
						record.data[WRF_TEMPERATURE_850HPA] = data_record->temperature_850hpa;
						if ( record.data[WRF_TEMPERATURE_850HPA] > parallel_dataset->max_value_vec[WRF_TEMPERATURE_850HPA] ) parallel_dataset->max_value_vec[WRF_TEMPERATURE_850HPA] = record.data[WRF_TEMPERATURE_850HPA];
						if ( record.data[WRF_TEMPERATURE_850HPA] < parallel_dataset->min_value_vec[WRF_TEMPERATURE_850HPA] ) parallel_dataset->min_value_vec[WRF_TEMPERATURE_850HPA] = record.data[WRF_TEMPERATURE_850HPA];
						/*record.data[WRF_WIND_Y_850HPA] = data_record->y_speed_850hpa;
						if ( record.data[WRF_WIND_Y_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
						if ( record.data[WRF_WIND_Y_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
						record.data[WRF_WIND_X_850HPA] = data_record->x_speed_850hpa;
						if ( record.data[WRF_WIND_X_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];
						if ( record.data[WRF_WIND_X_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];*/

						float yspeed = data_record->y_speed_850hpa;
						float xspeed = data_record->x_speed_850hpa;
						float speed = sqrt(pow(yspeed, 2) + pow(xspeed, 2));
						float angle;
						if ( xspeed != 0 ){
							angle = atan(yspeed / xspeed) / 3.14159 * 180;
							if ( xspeed < 0 ) angle = angle + 180;
							if ( angle < 0 ) angle += 360;
						} else {
							if ( yspeed >= 0 ) angle = 90;
							if ( yspeed < 0 ) angle = 270;
						}
						record.data[WRF_WIND_X_850HPA] = speed;
						record.data[WRF_WIND_Y_850HPA] = angle;
						if ( record.data[WRF_WIND_Y_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
						if ( record.data[WRF_WIND_Y_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
						if ( record.data[WRF_WIND_X_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];
						if ( record.data[WRF_WIND_X_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];

						record.data[WRF_HEIGHT_850HPA] = data_record->height_850hpa;
						if ( record.data[WRF_HEIGHT_850HPA] > parallel_dataset->max_value_vec[WRF_HEIGHT_850HPA] ) parallel_dataset->max_value_vec[WRF_HEIGHT_850HPA] = record.data[WRF_HEIGHT_850HPA];
						if ( record.data[WRF_HEIGHT_850HPA] < parallel_dataset->min_value_vec[WRF_HEIGHT_850HPA] ) parallel_dataset->min_value_vec[WRF_HEIGHT_850HPA] = record.data[WRF_HEIGHT_850HPA];
						record.data[WRF_RH_850HPA] = data_record->relative_humidity_850hpa;
						if ( record.data[WRF_RH_850HPA] > parallel_dataset->max_value_vec[WRF_RH_850HPA] ) parallel_dataset->max_value_vec[WRF_RH_850HPA] = record.data[WRF_RH_850HPA];
						if ( record.data[WRF_RH_850HPA] < parallel_dataset->min_value_vec[WRF_RH_850HPA] ) parallel_dataset->min_value_vec[WRF_RH_850HPA] = record.data[WRF_RH_850HPA];

						record.alpha = site_alpha_[i] * Ftime(current_time_ - t - 1);
						record.grid_index = i;

						switch ( record_set->type ){
						/*case WRF_T639:
							parallel_dataset->values[0].push_back(record);
							break;*/
						case WRF_EC_FINE:
							parallel_dataset->values[0].push_back(record);
							break;
						case WRF_NCEP:
							parallel_dataset->values[1].push_back(record);
							break;
						case WRF_JAPAN_GSM:
							parallel_dataset->values[2].push_back(record);
							break;
						default:
							break;
						}
					}
				} else {
					for ( int t = current_time_ - history_time_length_; t < current_time_; ++t ){
						WrfDataRecordSet* record_set = set_map->at(t);
						WrfDataRecord* his_record = record_set->his_values[i];
						WrfDataRecord* data_record = record_set->values[i];

						record.data[WRF_RAIN] = data_record->rain + his_record->rain;
						if ( record.data[WRF_RAIN] > parallel_dataset->max_value_vec[WRF_RAIN] ) parallel_dataset->max_value_vec[WRF_RAIN] = record.data[WRF_RAIN];
						if ( record.data[WRF_RAIN] < parallel_dataset->min_value_vec[WRF_RAIN] ) parallel_dataset->min_value_vec[WRF_RAIN] = record.data[WRF_RAIN];
						record.data[WRF_PRESSURE] = data_record->pressure + his_record->pressure;
						if ( record.data[WRF_PRESSURE] > parallel_dataset->max_value_vec[WRF_PRESSURE] ) parallel_dataset->max_value_vec[WRF_PRESSURE] = record.data[WRF_PRESSURE];
						if ( record.data[WRF_PRESSURE] < parallel_dataset->min_value_vec[WRF_PRESSURE] ) parallel_dataset->min_value_vec[WRF_PRESSURE] = record.data[WRF_PRESSURE];
						record.data[WRF_TEMPERATURE_850HPA] = data_record->temperature_850hpa + his_record->temperature_850hpa;
						if ( record.data[WRF_TEMPERATURE_850HPA] > parallel_dataset->max_value_vec[WRF_TEMPERATURE_850HPA] ) parallel_dataset->max_value_vec[WRF_TEMPERATURE_850HPA] = record.data[WRF_TEMPERATURE_850HPA];
						if ( record.data[WRF_TEMPERATURE_850HPA] < parallel_dataset->min_value_vec[WRF_TEMPERATURE_850HPA] ) parallel_dataset->min_value_vec[WRF_TEMPERATURE_850HPA] = record.data[WRF_TEMPERATURE_850HPA];

						float yspeed = data_record->y_speed_850hpa + his_record->y_speed_850hpa;
						float xspeed = data_record->x_speed_850hpa + his_record->x_speed_850hpa;
						float speed = sqrt(pow(yspeed, 2) + pow(xspeed, 2));
						float angle;
						if ( xspeed != 0 ){
							angle = atan(yspeed / xspeed) / 3.14159 * 180;
							if ( xspeed < 0 ) angle = angle + 180;
							if ( angle < 0 ) angle += 360;
						} else {
							if ( yspeed >= 0 ) angle = 90;
							if ( yspeed < 0 ) angle = 270;
						}
						record.data[WRF_WIND_X_850HPA] = speed;
						record.data[WRF_WIND_Y_850HPA] = angle;
						if ( record.data[WRF_WIND_Y_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
						if ( record.data[WRF_WIND_Y_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
						if ( record.data[WRF_WIND_X_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];
						if ( record.data[WRF_WIND_X_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];

						record.data[WRF_HEIGHT_850HPA] = data_record->height_850hpa + his_record->height_850hpa;
						if ( record.data[WRF_HEIGHT_850HPA] > parallel_dataset->max_value_vec[WRF_HEIGHT_850HPA] ) parallel_dataset->max_value_vec[WRF_HEIGHT_850HPA] = record.data[WRF_HEIGHT_850HPA];
						if ( record.data[WRF_HEIGHT_850HPA] < parallel_dataset->min_value_vec[WRF_HEIGHT_850HPA] ) parallel_dataset->min_value_vec[WRF_HEIGHT_850HPA] = record.data[WRF_HEIGHT_850HPA];
						record.data[WRF_RH_850HPA] = data_record->relative_humidity_850hpa + his_record->relative_humidity_850hpa;
						if ( record.data[WRF_RH_850HPA] > parallel_dataset->max_value_vec[WRF_RH_850HPA] ) parallel_dataset->max_value_vec[WRF_RH_850HPA] = record.data[WRF_RH_850HPA];
						if ( record.data[WRF_RH_850HPA] < parallel_dataset->min_value_vec[WRF_RH_850HPA] ) parallel_dataset->min_value_vec[WRF_RH_850HPA] = record.data[WRF_RH_850HPA];

						record.alpha = site_alpha_[i] * 0.8 * Ftime(current_time_ - t);
						record.grid_index = i;

						switch ( record_set->type ){
						/*case WRF_T639:
							parallel_dataset->values[0].push_back(record);
							break;*/
						case WRF_EC_FINE:
							parallel_dataset->values[0].push_back(record);
							break;
						case WRF_NCEP:
							parallel_dataset->values[1].push_back(record);
							break;
						case WRF_JAPAN_GSM:
							parallel_dataset->values[2].push_back(record);
							break;
						default:
							break;
						}
					}

					WrfDataRecordSet* record_set = set_map->at(current_time_);

					WrfDataRecord* data_record = record_set->values[i];
					record.data[WRF_RAIN] = data_record->rain;
					if ( record.data[WRF_RAIN] > parallel_dataset->max_value_vec[WRF_RAIN] ) parallel_dataset->max_value_vec[WRF_RAIN] = record.data[WRF_RAIN];
					if ( record.data[WRF_RAIN] < parallel_dataset->min_value_vec[WRF_RAIN] ) parallel_dataset->min_value_vec[WRF_RAIN] = record.data[WRF_RAIN];
					record.data[WRF_PRESSURE] = data_record->pressure;
					if ( record.data[WRF_PRESSURE] > parallel_dataset->max_value_vec[WRF_PRESSURE] ) parallel_dataset->max_value_vec[WRF_PRESSURE] = record.data[WRF_PRESSURE];
					if ( record.data[WRF_PRESSURE] < parallel_dataset->min_value_vec[WRF_PRESSURE] ) parallel_dataset->min_value_vec[WRF_PRESSURE] = record.data[WRF_PRESSURE];
					record.data[WRF_TEMPERATURE_850HPA] = data_record->temperature_850hpa;
					if ( record.data[WRF_TEMPERATURE_850HPA] > parallel_dataset->max_value_vec[WRF_TEMPERATURE_850HPA] ) parallel_dataset->max_value_vec[WRF_TEMPERATURE_850HPA] = record.data[WRF_TEMPERATURE_850HPA];
					if ( record.data[WRF_TEMPERATURE_850HPA] < parallel_dataset->min_value_vec[WRF_TEMPERATURE_850HPA] ) parallel_dataset->min_value_vec[WRF_TEMPERATURE_850HPA] = record.data[WRF_TEMPERATURE_850HPA];
					/*record.data[WRF_WIND_Y_850HPA] = data_record->y_speed_850hpa;
					if ( record.data[WRF_WIND_Y_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
					if ( record.data[WRF_WIND_Y_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
					record.data[WRF_WIND_X_850HPA] = data_record->x_speed_850hpa;
					if ( record.data[WRF_WIND_X_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];
					if ( record.data[WRF_WIND_X_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];*/

					float yspeed = data_record->y_speed_850hpa;
					float xspeed = data_record->x_speed_850hpa;
					float speed = sqrt(pow(yspeed, 2) + pow(xspeed, 2));
					float angle;
					if ( xspeed != 0 ){
						angle = atan(yspeed / xspeed) / 3.14159 * 180;
						if ( xspeed < 0 ) angle = angle + 180;
						if ( angle < 0 ) angle += 360;
					} else {
						if ( yspeed >= 0 ) angle = 90;
						if ( yspeed < 0 ) angle = 270;
					}
					record.data[WRF_WIND_X_850HPA] = speed;
					record.data[WRF_WIND_Y_850HPA] = angle;
					if ( record.data[WRF_WIND_Y_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
					if ( record.data[WRF_WIND_Y_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_Y_850HPA] = record.data[WRF_WIND_Y_850HPA];
					if ( record.data[WRF_WIND_X_850HPA] > parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->max_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];
					if ( record.data[WRF_WIND_X_850HPA] < parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] ) parallel_dataset->min_value_vec[WRF_WIND_X_850HPA] = record.data[WRF_WIND_X_850HPA];

					record.data[WRF_HEIGHT_850HPA] = data_record->height_850hpa;
					if ( record.data[WRF_HEIGHT_850HPA] > parallel_dataset->max_value_vec[WRF_HEIGHT_850HPA] ) parallel_dataset->max_value_vec[WRF_HEIGHT_850HPA] = record.data[WRF_HEIGHT_850HPA];
					if ( record.data[WRF_HEIGHT_850HPA] < parallel_dataset->min_value_vec[WRF_HEIGHT_850HPA] ) parallel_dataset->min_value_vec[WRF_HEIGHT_850HPA] = record.data[WRF_HEIGHT_850HPA];
					record.data[WRF_RH_850HPA] = data_record->relative_humidity_850hpa;
					if ( record.data[WRF_RH_850HPA] > parallel_dataset->max_value_vec[WRF_RH_850HPA] ) parallel_dataset->max_value_vec[WRF_RH_850HPA] = record.data[WRF_RH_850HPA];
					if ( record.data[WRF_RH_850HPA] < parallel_dataset->min_value_vec[WRF_RH_850HPA] ) parallel_dataset->min_value_vec[WRF_RH_850HPA] = record.data[WRF_RH_850HPA];

					record.alpha = site_alpha_[i];
					record.grid_index = i;

					switch ( record_set->type ){
					/*case WRF_T639:
						parallel_dataset->values[0].push_back(record);
						break;*/
					case WRF_EC_FINE:
						parallel_dataset->values[0].push_back(record);
						break;
					case WRF_NCEP:
						parallel_dataset->values[1].push_back(record);
						break;
					case WRF_JAPAN_GSM:
						parallel_dataset->values[2].push_back(record);
						break;
					default:
						break;
					}
				}
			}
		}
	}

	if ( !is_absolute ){
		for ( int i = 0; i < parallel_dataset->max_value_vec.size(); ++i ){
			float max_abs = abs(parallel_dataset->max_value_vec[i]);
			if ( abs(parallel_dataset->min_value_vec[i]) > max_abs )
				max_abs = abs(parallel_dataset->min_value_vec[i]);
			parallel_dataset->max_value_vec[i] = max_abs;
			parallel_dataset->min_value_vec[i] = -1 * max_abs;
		}
	}
}

WrfLineChartDataSet* WrfDataManager::GetLineChartDataSet(WrfGeneralDataStampType data_type, bool is_absolute){

	WrfLineChartDataSet* line_chart_dataset;

	if ( is_absolute )
		line_chart_dataset = absolute_line_chart_dataset_;
	else 
		line_chart_dataset = bias_line_chart_dataset_;

	if ( line_chart_dataset != NULL ) delete line_chart_dataset;
	line_chart_dataset = new WrfLineChartDataSet;
	
	GetLineChartDataSet(data_type, selected_grid_index_, is_absolute, line_chart_dataset);

	if ( is_absolute )
		absolute_line_chart_dataset_ = line_chart_dataset;
	else
		bias_line_chart_dataset_ = line_chart_dataset;

	return line_chart_dataset;
}

WrfLineChartDataSet* WrfDataManager::GetComparedLineChartDataSet(WrfGeneralDataStampType data_type, bool is_absolute){

	WrfLineChartDataSet* line_chart_dataset;

	if ( is_absolute )
		line_chart_dataset = compared_absolute_line_chart_dataset_;
	else 
		line_chart_dataset = compared_bias_line_chart_dataset_;

	if ( line_chart_dataset != NULL ) delete line_chart_dataset;
	line_chart_dataset = new WrfLineChartDataSet;

	GetLineChartDataSet(data_type, compared_grid_index_, is_absolute, line_chart_dataset);

	if ( is_absolute )
		absolute_line_chart_dataset_ = compared_absolute_line_chart_dataset_;
	else
		bias_line_chart_dataset_ = compared_bias_line_chart_dataset_;

	return line_chart_dataset;
}

void WrfDataManager::GetLineChartDataSet(WrfGeneralDataStampType data_type, std::vector< bool >& selected_index, bool is_absolute, WrfLineChartDataSet* line_chart_dataset){
	line_chart_dataset->time_length = history_time_length_ + 1;
	line_chart_dataset->max_value = -1e10;
	line_chart_dataset->min_value = 1e10;
	line_chart_dataset->values.resize(numerical_data_.size() + 1);

	std::vector< std::vector< float > >* temp_model_weight = grid_model_weight_map_[current_time_ - 1];
	double average_value = 0;
	double average_bias_value = 0;

	int longi_grid_size = longitude_grid_number_ / grid_size_;
	int lati_grid_size = latitude_grid_number_ / grid_size_;
	for ( int i = 0; i < longitude_grid_number_ * latitude_grid_number_; ++i ){
		int temp_longi = i % longitude_grid_number_;
		temp_longi /= grid_size_;
		int temp_lati = i / longitude_grid_number_;
		temp_lati /= grid_size_;
		if ( temp_longi < longi_grid_size && temp_lati < lati_grid_size && selected_index[i] ){
			int temp_grid_index = temp_lati * longi_grid_size + temp_longi;
			std::vector< float > temp_average;
			temp_average.resize(history_time_length_ + 1, 0);
			float temp_alpha = 0;
			for ( int k = 0; k < bias_set_vec_.size(); ++k ){
				RecordSetMap* set_map = bias_set_vec_.at(k);
				LineChartRecord* record = new LineChartRecord;

				for ( int t = current_time_ - history_time_length_; t <= current_time_; ++t ){

					if ( !is_absolute ){
						WrfDataRecord* data_record = set_map->at(t)->values[i];
						switch ( data_type ){
						case WRF_RAIN:
							record->values.push_back(data_record->rain);
							break;
						case WRF_PRESSURE:
							record->values.push_back(data_record->pressure);
							break;
						case WRF_TEMPERATURE_850HPA:
							record->values.push_back(data_record->temperature_850hpa);
							break;
						case WRF_HEIGHT_850HPA:
							record->values.push_back(data_record->height_850hpa);
							break;
						case WRF_WIND_X_850HPA:
							{
								float yspeed = data_record->y_speed_850hpa;
								float xspeed = data_record->x_speed_850hpa;
								float speed = sqrt(pow(yspeed, 2) + pow(xspeed, 2));
								record->values.push_back(speed);
							}
							break;
						case WRF_WIND_Y_850HPA:
							{
								float yspeed = data_record->y_speed_850hpa;
								float xspeed = data_record->x_speed_850hpa;
								float angle;
								if ( xspeed != 0 ){
									angle = atan(yspeed / xspeed) / 3.14159 * 180;
									if ( xspeed < 0 ) angle = angle + 180;
									if ( angle < 0 ) angle += 360;
								} else {
									if ( yspeed >= 0 ) angle = 90;
									if ( yspeed < 0 ) angle = 270;
								}
								record->values.push_back(angle);
							}
							break;
						case WRF_RH_850HPA:
							record->values.push_back(data_record->relative_humidity_850hpa);
							break;
						default:
							exit(0);
							break;
						}

						if ( t == current_time_ - 1 ){
							average_bias_value += record->values[record->values.size() - 1] * temp_model_weight->at(temp_grid_index)[k];
						} else if ( t == current_time_ ){
							average_value += record->values[record->values.size() - 1] * temp_model_weight->at(temp_grid_index)[k];
						}

						if ( t != current_time_ ){
							if ( line_chart_dataset->max_value < record->values[record->values.size() - 1] ) line_chart_dataset->max_value = record->values[record->values.size() - 1];
							if ( line_chart_dataset->min_value > record->values[record->values.size() - 1] ) line_chart_dataset->min_value = record->values[record->values.size() - 1];
						}
					} else {
						WrfDataRecord* data_record = set_map->at(t)->values[i];
						WrfDataRecord* his_record = set_map->at(t)->his_values[i];
						switch ( data_type ){
						case WRF_RAIN:
							record->values.push_back(data_record->rain + his_record->rain);
							break;
						case WRF_PRESSURE:
							record->values.push_back(data_record->pressure + his_record->pressure);
							break;
						case WRF_TEMPERATURE_850HPA:
							record->values.push_back(data_record->temperature_850hpa + his_record->temperature_850hpa);
							break;
						case WRF_HEIGHT_850HPA:
							record->values.push_back(data_record->height_850hpa + his_record->height_850hpa);
							break;
						case WRF_WIND_X_850HPA:
							{
								float xspeed = data_record->x_speed_850hpa + his_record->x_speed_850hpa;
								float yspeed = data_record->y_speed_850hpa + his_record->y_speed_850hpa;
								float speed = sqrt(pow(yspeed, 2) + pow(xspeed, 2));
								record->values.push_back(speed);
							}
							break;
						case WRF_WIND_Y_850HPA:
							{
								float xspeed = data_record->x_speed_850hpa + his_record->x_speed_850hpa;
								float yspeed = data_record->y_speed_850hpa + his_record->y_speed_850hpa;
								float angle;
								if ( xspeed != 0 ){
									angle = atan(yspeed / xspeed) / 3.14159 * 180;
									if ( xspeed < 0 ) angle = angle + 180;
									if ( angle < 0 ) angle += 360;
								} else {
									if ( yspeed >= 0 ) angle = 90;
									if ( yspeed < 0 ) angle = 270;
								}
								record->values.push_back(angle);
							}
							break;
						case WRF_RH_850HPA:
							record->values.push_back(data_record->relative_humidity_850hpa + his_record->relative_humidity_850hpa);
							break;
						default:
							exit(0);
							break;
						}

						if ( line_chart_dataset->max_value < record->values[record->values.size() - 1] ) line_chart_dataset->max_value = record->values[record->values.size() - 1];
						if ( line_chart_dataset->min_value > record->values[record->values.size() - 1] ) line_chart_dataset->min_value = record->values[record->values.size() - 1];
					}

					temp_average[t - current_time_ + history_time_length_] += record->values[record->values.size() - 1] * temp_model_weight->at(temp_grid_index)[k];
				}

				record->alpha = site_alpha_[i];
				record->grid_index = i;
				temp_alpha += site_alpha_[i] * 1.0 / bias_set_vec_.size();

				line_chart_dataset->values[k].push_back(record);
			}
			LineChartRecord* weight_record = new LineChartRecord;
			weight_record->alpha = temp_alpha;
			for ( int k = 0; k < temp_average.size(); ++k ) weight_record->values.push_back(temp_average[k]);
			weight_record->grid_index = i;
			line_chart_dataset->values[bias_set_vec_.size()].push_back(weight_record);
		}
	}

	if ( !is_absolute ){
		average_bias_value /= line_chart_dataset->values[0].size();
		average_value /= line_chart_dataset->values[0].size();
		//line_chart_dataset->center_absolute_value = average_value - average_bias_value;
		line_chart_dataset->center_absolute_value = 0;
		for ( int i = 0; i < line_chart_dataset->values.size(); ++i )
			for ( int j = 0; j < line_chart_dataset->values[i].size(); ++j ){
				float current_value = line_chart_dataset->values[i][j]->values[line_chart_dataset->values[i][j]->values.size() - 1];
				if ( current_value - line_chart_dataset->center_absolute_value < line_chart_dataset->min_value ) 
					line_chart_dataset->min_value = current_value - line_chart_dataset->center_absolute_value;
				if ( current_value - line_chart_dataset->center_absolute_value > line_chart_dataset->max_value )
					line_chart_dataset->max_value = current_value - line_chart_dataset->center_absolute_value;
				line_chart_dataset->values[i][j]->values[line_chart_dataset->values[i][j]->values.size() - 1] -= line_chart_dataset->center_absolute_value;
			}
	}


	line_chart_dataset->colors.resize(3 * 4);
	line_chart_dataset->colors[0] = 0.0;
	line_chart_dataset->colors[1] = 1.0;
	line_chart_dataset->colors[2] = 0.0;

	line_chart_dataset->colors[3] = 0.0;
	line_chart_dataset->colors[4] = 0.0;
	line_chart_dataset->colors[5] = 1.0;

	line_chart_dataset->colors[6] = 0.0;
	line_chart_dataset->colors[7] = 1.0;
	line_chart_dataset->colors[8] = 1.0;

	line_chart_dataset->colors[9] = 1.0;
	line_chart_dataset->colors[10] = 1.0;
	line_chart_dataset->colors[11] = 0.0;

	/*line_chart_dataset->colors[12] = 0.0;
	line_chart_dataset->colors[13] = 1.0;
	line_chart_dataset->colors[14] = 1.0;*/

	//line_chart_dataset->line_names.push_back("T639");
	line_chart_dataset->line_names.push_back("EC_FINE");
	line_chart_dataset->line_names.push_back("NCEP");
	line_chart_dataset->line_names.push_back("JAPAN_GSM");
	line_chart_dataset->line_names.push_back("Weighted Average");
}

WrfHistogramDataSet* WrfDataManager::GetHistogramDataSet(){
	if ( histogram_dataset_ != NULL ) delete histogram_dataset_;
	histogram_dataset_ = new WrfHistogramDataSet;

	for ( int t = 0; t < history_time_length_; ++t ){
		std::vector< std::vector< float > > grid_weight;
		std::vector< std::vector< float > >* current_grid_model_weight = grid_model_weight_map_[current_time_ - (history_time_length_ - t)];
		for ( int i = 0; i < selected_patch_index_.size(); ++i ){
			if ( selected_patch_index_[i] != true ) continue;
			std::vector< float > temp_model_weight;
			temp_model_weight.resize(current_grid_model_weight->at(i).size());
			temp_model_weight.assign(current_grid_model_weight->at(i).begin(), current_grid_model_weight->at(i).end());
			grid_weight.push_back(temp_model_weight);
		}
		histogram_dataset_->grid_model_weight.push_back(grid_weight);
	}

	for ( int i = 0; i < selected_patch_index_.size(); ++i ){
		if ( selected_patch_index_[i] != true ) continue;
		histogram_dataset_->grid_index.push_back(i);
	}

	/*histogram_dataset_->grid_index.push_back(1);
	srand((unsigned int)time(0));
	for ( int t = 0; t < 4; ++t ){
		std::vector< std::vector< float > > grid_weight;
		float base = (float)rand() / RAND_MAX;
		for ( int i = 0; i < 40; ++i ){
			std::vector< float > temp_model_weight;
			for ( int j = 0; j < 4; ++j ){
				temp_model_weight.push_back((float)rand() / RAND_MAX * 0.7f + (float)rand() / RAND_MAX * 0.7);
			}
			grid_weight.push_back(temp_model_weight);
		}
		histogram_dataset_->grid_model_weight.push_back(grid_weight);
	}*/

	histogram_dataset_->model_rgb_color.resize(3 * 3);
	histogram_dataset_->model_rgb_color[0] = 0.0;
	histogram_dataset_->model_rgb_color[1] = 1.0;
	histogram_dataset_->model_rgb_color[2] = 0.0;

	histogram_dataset_->model_rgb_color[3] = 0.0;
	histogram_dataset_->model_rgb_color[4] = 0.0;
	histogram_dataset_->model_rgb_color[5] = 1.0;

	histogram_dataset_->model_rgb_color[6] = 0.0;
	histogram_dataset_->model_rgb_color[7] = 1.0;
	histogram_dataset_->model_rgb_color[8] = 1.0;

	/*histogram_dataset_->model_rgb_color[9] = 1.0;
	histogram_dataset_->model_rgb_color[10] = 1.0;
	histogram_dataset_->model_rgb_color[11] = 0.0;*/

	return histogram_dataset_;
}

void WrfDataManager::LoadDefaultData(int current, int day_length){
	current_time_ = current;

	int temp_history_length = history_time_length_;

	history_time_length_ = day_length;
	// Load Historical Data
	LoadScene();

	// Load Numerical Data
	//LoadT639();
	LoadEcFine();
	LoadNcep();
	LoadJapanGsm();

	NormalizeData();

	UpdateSiteAlphaMap();

	GenerateBiasMap(day_length);

	history_time_length_ = temp_history_length;
}

void WrfDataManager::NormalizeData(){
	for ( int i = 0; i < numerical_data_.size(); ++i ){
		DataStampMap* stamp_map = numerical_data_.at(i);
		DataStampMap::iterator iter = stamp_map->begin();
		while ( iter != stamp_map->end() ){
			std::vector< WrfDataStamp* >* stamp_vec = iter->second;
			for ( int j = 0; j < stamp_vec->size(); ++j ){
				WrfDataStamp* data_stamp = stamp_vec->at(j);
				WrfGridDataStamp* grid_stamp = dynamic_cast< WrfGridDataStamp* >(data_stamp);
				for ( int k = 0; k < grid_stamp->values.size(); ++k ){
					if ( grid_stamp->values[k] > attribute_max_value_[grid_stamp->data_type()] )
						attribute_max_value_[grid_stamp->data_type()] = grid_stamp->values[k];
					if ( grid_stamp->values[k] < attribute_min_value_[grid_stamp->data_type()] )
						attribute_min_value_[grid_stamp->data_type()] = grid_stamp->values[k];
				}
			}
			iter++;
		}
	}
	
	DataStampMap::iterator iter = historical_data_.begin();
	while ( iter != historical_data_.end() ){
		std::vector< WrfDataStamp* >* stamp_vec = iter->second;
		for ( int i = 0; i < stamp_vec->size(); ++i ){
			WrfDataStamp* data_stamp = stamp_vec->at(i);
			switch ( data_stamp->format_type() ){
			case WrfDataFormatType::WRF_FORMAT_THREE:
				{
					WrfDiscreteDataStamp* discrete_stamp = dynamic_cast< WrfDiscreteDataStamp* >(data_stamp);
					for ( int k = 0; k < discrete_stamp->values.size(); ++k ){
						switch ( discrete_stamp->data_type() ){
						case WRF_RAIN:
						case WRF_PRESSURE:
						case WRF_TEMPERATURE_850HPA:
						case WRF_HEIGHT_850HPA:
						case WRF_RH_850HPA:
							if ( discrete_stamp->values[k].value2 > attribute_max_value_[discrete_stamp->data_type()] )
								attribute_max_value_[discrete_stamp->data_type()] = discrete_stamp->values[k].value2;
							if ( discrete_stamp->values[k].value2 < attribute_min_value_[discrete_stamp->data_type()] )
								attribute_min_value_[discrete_stamp->data_type()] = discrete_stamp->values[k].value2;
							break;
						case WRF_WIND_850HPA:
							if ( discrete_stamp->values[k].value1 > attribute_max_value_[discrete_stamp->data_type()] )
								attribute_max_value_[discrete_stamp->data_type()] = discrete_stamp->values[k].value1;
							if ( discrete_stamp->values[k].value1 < attribute_min_value_[discrete_stamp->data_type()] )
								attribute_min_value_[discrete_stamp->data_type()] = discrete_stamp->values[k].value1;
							if ( discrete_stamp->values[k].value2 > attribute_max_value_[discrete_stamp->data_type()] )
								attribute_max_value_[discrete_stamp->data_type()] = discrete_stamp->values[k].value2;
							if ( discrete_stamp->values[k].value2 < attribute_min_value_[discrete_stamp->data_type()] )
								attribute_min_value_[discrete_stamp->data_type()] = discrete_stamp->values[k].value2;
							break;
						default:
							exit(0);
							break;
						}
					}
				}
				break;
			default:
				exit(0);
				break;
			}
		}
		iter++;
	}

	/*for ( int i = 0; i < numerical_data_.size(); ++i ){
		DataStampMap* stamp_map = numerical_data_.at(i);
		DataStampMap::iterator iter = stamp_map->begin();
		while ( iter != stamp_map->end() ){
			std::vector< WrfDataStamp* >* stamp_vec = iter->second;
			for ( int j = 0; j < stamp_vec->size(); ++j ){
				WrfDataStamp* data_stamp = stamp_vec->at(j);
				WrfGridDataStamp* grid_stamp = dynamic_cast< WrfGridDataStamp* >(data_stamp);
				for ( int k = 0; k < grid_stamp->values.size(); ++k ){
					grid_stamp->values[k] = (grid_stamp->values[k] - attribute_min_value_[grid_stamp->data_type()]) / (attribute_max_value_[grid_stamp->data_type()] - attribute_min_value_[grid_stamp->data_type()]);
				}
			}
			iter++;
		}
	}

	iter = historical_data_.begin();
	while ( iter != historical_data_.end() ){
		std::vector< WrfDataStamp* >* stamp_vec = iter->second;
		for ( int i = 0; i < stamp_vec->size(); ++i ){
			WrfDataStamp* data_stamp = stamp_vec->at(i);
			switch ( data_stamp->format_type() ){
			case WrfDataFormatType::WRF_FORMAT_THREE:
				{
					WrfDiscreteDataStamp* discrete_stamp = dynamic_cast< WrfDiscreteDataStamp* >(data_stamp);
					for ( int k = 0; k < discrete_stamp->values.size(); ++k ){
						discrete_stamp->values[k].value2 = (discrete_stamp->values[k].value2 - attribute_min_value_[discrete_stamp->data_type()]) / (attribute_max_value_[discrete_stamp->data_type()] - attribute_min_value_[discrete_stamp->data_type()]);
					}
				}
				break;
			case WrfDataFormatType::WRF_FORMAT_FOUR:
				{
					WrfDataStamp* data_stamp = stamp_vec->at(i);
					WrfGridDataStamp* grid_stamp = dynamic_cast< WrfGridDataStamp* >(data_stamp);
					for ( int k = 0; k < grid_stamp->values.size(); ++k ){
						grid_stamp->values[k] = (grid_stamp->values[k] - attribute_min_value_[grid_stamp->data_type()]) / (attribute_max_value_[grid_stamp->data_type()] - attribute_min_value_[grid_stamp->data_type()]);
					}
				}
				break;
			default:
				break;
			}
		}
		iter++;
	}*/
}

void WrfDataManager::UpdateSiteAlphaMap(){
	memset(site_alpha_.data(), 0, site_alpha_.size() * sizeof(float));

	float radius = 2.5;
	int radius_grid_size = (int)(radius / longitude_grid_space_) + 1;
	float max_site_value = 0;
	for ( int t = 0; t < history_time_length_; ++t ){
		int temp_time = current_time_ - t - 1;
		std::vector< WrfDataStamp* >* stamp_vec = historical_data_[temp_time];

		for ( int i = 0; i < stamp_vec->size(); ++i ){
			WrfDiscreteDataStamp* discrete_stamp = dynamic_cast< WrfDiscreteDataStamp* >(stamp_vec->at(i));
			if ( discrete_stamp != NULL ){
				for ( int j = 0; j < discrete_stamp->values.size(); ++j ){
					float temp_longitude = discrete_stamp->values.at(j).longitude;
					float temp_latitude = discrete_stamp->values.at(j).latitude;

					int longitude_index = (int)((temp_longitude - start_longitude_) / longitude_grid_space_ + 0.5);
					int latitude_index = (int)((temp_latitude - start_latitude_) / latitude_grid_space_ + 0.5);

					for ( int lati = latitude_index - radius_grid_size; lati < latitude_index + radius_grid_size; ++lati ){
						if ( lati < 0 || lati >= latitude_grid_number_ ) continue;
						for ( int longi = longitude_index - radius_grid_size; longi < longitude_index + radius_grid_size; ++longi){
							if ( longi < 0 || longi >= longitude_grid_number_ ) continue;
							float temp1 = start_longitude_ + longi * longitude_grid_space_;
							float temp2 = start_latitude_ + lati * latitude_grid_space_;
							float dis = sqrt(pow(temp1 - temp_longitude, 2) + pow(temp2 - temp_latitude, 2));
							site_alpha_[lati * longitude_grid_number_ + longi] += exp(-1 * dis / radius) * exp(-1.0 * t / 3);
							if ( site_alpha_[lati * longitude_grid_number_ + longi] > max_site_value )
								max_site_value = site_alpha_[lati * longitude_grid_number_ + longi];
						}
					}
				}
			}
		}
	}

	for ( int i = 0; i < site_alpha_.size(); ++i ) site_alpha_[i] /= max_site_value;
}

void WrfDataManager::GenerateBiasMap(int day_length){
	int temp_history_length = history_time_length_;
	history_time_length_ = day_length;

	GenerateBaseMaps();

	for ( int i = 0; i < numerical_data_.size(); ++i ){
		RecordSetMap* record_set_map = new RecordSetMap;
		DataStampMap* stamp_map = numerical_data_.at(i);

		for ( int t = current_time_ - history_time_length_; t < current_time_; ++t ){
			std::vector< WrfDataStamp* >* numerical_stamps = stamp_map->at(t);
			DataStampMap::iterator his_map_iter = historical_data_.find(t);
			if ( his_map_iter == historical_data_.end() ){
				std::cout << "Can't match t639 data with historical data" << std::endl;
				break;
			}
			std::vector< WrfDataStamp* >* historical_stamps = his_map_iter->second;
			std::vector< WrfDataStamp* >* base_stamp = base_map_data_.find(t)->second;

			WrfDataRecordSetType type;
			switch ( i ){
			/*case 0:
				type = WRF_T639;
				break;*/
			case 0:
				type = WRF_EC_FINE;
				break;
			case 1:
				type = WRF_NCEP;
				break;
			case 2:
				type = WRF_JAPAN_GSM;
				break;
			default:
				break;
			}

			WrfDataRecordSet* record_set = data_converter_->Convert(*numerical_stamps, *historical_stamps, *base_stamp, type);	
			record_set_map->insert(RecordSetMap::value_type(t, record_set));
		}
		bias_set_vec_.push_back(record_set_map);
	}


	for ( int i = 0; i < numerical_data_.size(); ++i ){
		RecordSetMap* record_set_map = bias_set_vec_[i];
		DataStampMap* stamp_map = numerical_data_.at(i);
		std::vector< WrfDataStamp* >* numerical_stamps = stamp_map->at(current_time_);

		std::vector< WrfDataStamp* > historical_stamps;
		std::vector< WrfDataStamp* > base_stamp;

		WrfDataRecordSetType type;
		switch ( i ){
		/*case 0:
			type = WRF_T639;
			break;*/
		case 0:
			type = WRF_EC_FINE;
			break;
		case 1:
			type = WRF_NCEP;
			break;
		case 2:
			type = WRF_JAPAN_GSM;
			break;
		default:
			break;
		}

		WrfDataRecordSet* record_set = data_converter_->Convert(*numerical_stamps, historical_stamps, base_stamp, type);	
		record_set_map->insert(RecordSetMap::value_type(current_time_, record_set));
	}
	

	// Generate the grid based model weight
	int temp_latitude_num = latitude_grid_number_ / grid_size_;
	int temp_longitude_num = longitude_grid_number_ / grid_size_;
	std::vector< std::vector< float > >* grid_model_weight = new std::vector< std::vector< float > >;
	grid_model_weight->resize(temp_latitude_num * temp_longitude_num, model_weight_);
	grid_model_weight_map_.insert(WeightMap::value_type(0, grid_model_weight));

	/*std::vector< float > temp_covariance_vec;
	temp_covariance_vec.resize(model_weight_.size() * model_weight_.size(), 0);
	covariance_vec_.resize(temp_longitude_num * temp_latitude_num, temp_covariance_vec);*/

	//UpdateGridModelWeight(current_time_);
	SystemTimer::GetInstance()->BeginTimer();
	for ( int t = current_time_ - history_time_length_; t < current_time_; ++t ) UpdateSimpleGridModelWeight(t);
	SystemTimer::GetInstance()->EndTimer();
	std::cout << "Time for updating model weight: " << std::endl;
	SystemTimer::GetInstance()->PrintTime();

	history_time_length_ = temp_history_length;
}

void WrfDataManager::UpdateSimpleGridModelWeight(int time){
	std::vector< float > bias_sum;
	bias_sum.resize(numerical_data_.size());

	std::vector< std::vector< float > >* grid_model_weight = new std::vector< std::vector< float > >;
	std::vector< std::vector< float > >* template_weight;
	if ( grid_model_weight_map_.find(time - 1) != grid_model_weight_map_.end() ){
		template_weight = grid_model_weight_map_[time - 1];
	} else {
		template_weight = grid_model_weight_map_[0];
	}
	 
	grid_model_weight->resize(template_weight->size());
	grid_model_weight->assign(template_weight->begin(), template_weight->end());

	int temp_longitude_index;
	int temp_latitude_index;
	int temp_grid_index;
	temp_latitude_index = 0;
	temp_grid_index = 0;
	for ( int i = 0; i < latitude_grid_number_ / grid_size_; ++i ){
		temp_longitude_index = 0;
		for ( int j = 0; j < longitude_grid_number_ / grid_size_; ++j ){
			memset(bias_sum.data(), 0, bias_sum.size() * sizeof(float));

			for ( int lati = temp_latitude_index; lati < temp_latitude_index + grid_size_; ++lati )
				for ( int longi = temp_longitude_index; longi < temp_longitude_index + grid_size_; ++longi ){
					for ( int k = 0; k < bias_set_vec_.size(); ++k ){
						RecordSetMap* record_map = bias_set_vec_.at(k);
						WrfDataRecordSet* record_set = record_map->at(time);
						int temp_index = lati * longitude_grid_number_ + longi;
						WrfDataRecord* temp_record = record_set->values[temp_index];
						bias_sum[k] += abs(temp_record->rain) * attribute_weight_[WRF_RAIN];
						bias_sum[k] += abs(temp_record->height_850hpa) * attribute_weight_[WRF_HEIGHT_850HPA];
						bias_sum[k] += abs(temp_record->pressure) * attribute_weight_[WRF_PRESSURE];
						bias_sum[k] += abs(temp_record->x_speed_850hpa) * attribute_weight_[WRF_WIND_X_850HPA];
						bias_sum[k] += abs(temp_record->y_speed_850hpa) * attribute_weight_[WRF_WIND_Y_850HPA];
						bias_sum[k] += abs(temp_record->temperature_850hpa) * attribute_weight_[WRF_TEMPERATURE_850HPA];
					}
				}

			float sum_all = 0;
			for ( int k = 0; k < bias_set_vec_.size(); ++k ) sum_all += bias_sum[k];
			if ( sum_all != 0 ){
				for ( int k = 0; k < bias_set_vec_.size(); ++k ) bias_sum[k] = (sum_all - bias_sum[k]) / sum_all;
				sum_all = 0;
				for ( int k = 0; k < bias_set_vec_.size(); ++k ) sum_all += bias_sum[k];
				for ( int k = 0; k < bias_set_vec_.size(); ++k ) bias_sum[k] /= sum_all;

				for ( int k = 0; k < bias_set_vec_.size(); ++k ) grid_model_weight->at(temp_grid_index)[k] += bias_sum[k];
				sum_all = 0;
				for ( int k = 0; k < bias_set_vec_.size(); ++k ) sum_all += grid_model_weight->at(temp_grid_index)[k];
				for ( int k = 0; k < bias_set_vec_.size(); ++k ) grid_model_weight->at(temp_grid_index)[k] /= sum_all;
			}
			

			temp_longitude_index += grid_size_;
			temp_grid_index++;
		}
		temp_latitude_index += grid_size_;
	}

	grid_model_weight_map_.insert(WeightMap::value_type(time, grid_model_weight));
}

void WrfDataManager::UpdateGridModelWeight(int time){
	/*std::vector< WrfDataStamp* >* base_maps = base_map_data_.at(time);
	
	std::vector< float > W, V, C_current, SIGMA, R, A, AT, tempXR, tempXRX, tempRX, invSIGMA, tempAGain, YPredict, tempASigma, tempASigmaA;
	W.resize(model_weight_.size() * model_weight_.size());
	V.resize(base_maps->size() * base_maps->size());
	R.resize(model_weight_.size() * model_weight_.size());
	SIGMA.resize(base_maps->size() * base_maps->size());
	invSIGMA.resize(base_maps->size() * base_maps->size());
	A.resize(model_weight_.size() * base_maps->size());
	AT.resize(model_weight_.size() * base_maps->size());
	tempXR.resize(base_maps->size() * model_weight_.size());
	tempRX.resize(model_weight_.size() * base_maps->size());
	tempXRX.resize(base_maps->size() * base_maps->size());
	tempAGain.resize(model_weight_.size());
	YPredict.resize(base_maps->size());
	tempASigma.resize(model_weight_.size() * base_maps->size());
	tempASigmaA.resize(model_weight_.size() * model_weight_.size());

	lapack_int info, m, n, lda, ldb, nrhs;
	std::vector< float > b;
	std::vector< float > a, aT;
	std::vector< float > copy_b, copy_a;

	b.resize(base_maps->size());
	a.resize(base_maps->size() * numerical_data_.size());
	aT.resize(base_maps->size() * numerical_data_.size());
	copy_a.resize(a.size());
	copy_b.resize(b.size());
	m = base_maps->size();
	n = numerical_data_.size();
	nrhs = 1;
	lda = numerical_data_.size();
	ldb = 1;

	float temp_longitude_index;
	float temp_latitude_index;
	int temp_grid_index;
	temp_latitude_index = 0;
	temp_grid_index = 0;
	for ( int i = 0; i < latitude_grid_number_ / grid_size_; ++i ){
		temp_longitude_index = 0;
		for ( int j = 0; j < longitude_grid_number_ / grid_size_; ++j ){
			memset(b.data(), 0, b.size() * sizeof(double));
			memset(a.data(), 0, a.size() * sizeof(double));

			for ( int k = 0; k < base_maps->size(); ++k ){
				for ( int lati = temp_latitude_index; lati < temp_latitude_index + grid_size_; ++lati )
					for ( int longi = temp_longitude_index; longi < temp_longitude_index + grid_size_; ++longi ){
						WrfGridDataStamp* base_grid_map = dynamic_cast< WrfGridDataStamp* >(base_maps->at(k));
						b[k] += base_grid_map->values[temp_latitude_index * longitude_grid_number_ + temp_longitude_index] * attribute_weight_[k];

						for ( int m = 0; m < numerical_data_.size(); ++m ){
							std::vector< WrfDataStamp* >* num_data = numerical_data_.at(m)->at(time);
							WrfGridDataStamp* num_grid_stamp = dynamic_cast< WrfGridDataStamp* >(num_data->at(k));
							a[k * numerical_data_.size() + m] += num_grid_stamp->values[temp_latitude_index * longitude_grid_number_ + temp_longitude_index] * attribute_weight_[k];
						}
					}
			}
			for ( int k = 0; k < b.size(); ++k ) b[k] /= (grid_size_ * grid_size_);
			for ( int k = 0; k < a.size(); ++k ) a[k] /= (grid_size_ * grid_size_);
			copy_a.assign(a.begin(), a.end());
			copy_b.assign(b.begin(), b.end());

			info = LAPACKE_sgels(LAPACK_ROW_MAJOR, 'N', m, n, nrhs, copy_a.data(), lda, copy_b.data(), ldb);

			int current_grid_index = i * (longitude_grid_number_ / grid_size_) + j;
			memset(W.data(), 0, W.size() * sizeof(float));
			for ( int k = 0; k < model_weight_.size(); ++k ) W[k * model_weight_.size() + k] = pow(grid_model_weight_[current_grid_index][k] - copy_b[k], 2);
			for ( int k = 0; k < R.size(); ++k ) R[k] = covariance_vec_[current_grid_index][k] + W[k];
			memset(V.data(), 0, V.size() * sizeof(float));
			for ( int k = 0; k < base_maps->size(); ++k ) {
				V[k * base_maps->size() + k] = b[k];
				for ( int m = 0; m < numerical_data_.size(); ++m )
					V[k * base_maps->size() + k] -= a[k * numerical_data_.size() + m] * attribute_weight_[k] * model_weight_[m];
				V[k * base_maps->size() + k] /= grid_size_ * grid_size_ - base_maps->size() - 1;
			}
			
			MatrixT(a.data(), model_weight_.size(), base_maps->size(), aT.data());
			MatrixMul(a.data(), base_maps->size(), model_weight_.size(), R.data(), model_weight_.size(), model_weight_.size(), tempXR.data());
			MatrixMul(tempXR.data(), base_maps->size(), model_weight_.size(), aT.data(), model_weight_.size(), base_maps->size(), tempXRX.data());
			memset(SIGMA.data(), 0, SIGMA.size() * sizeof(float));
			for ( int k = 0; k < SIGMA.size(); ++k ) SIGMA[k] = tempXRX[k] + V[k];
			MatrixInv(SIGMA.data(), base_maps->size(), base_maps->size(), invSIGMA.data());
			MatrixMul(R.data(), model_weight_.size(), model_weight_.size(), aT.data(), model_weight_.size(), base_maps->size(), tempRX.data());
			MatrixMul(tempRX.data(), model_weight_.size(), base_maps->size(), invSIGMA.data(), base_maps->size(), base_maps->size(), A.data());
			MatrixMul(a.data(), base_maps->size(), model_weight_.size(), grid_model_weight_[temp_grid_index].data(), model_weight_.size(), 1, YPredict.data());
			for ( int k = 0; k < YPredict.size(); ++k ) b[k] -= YPredict[k];
			MatrixMul(A.data(), model_weight_.size(), base_maps->size(), b.data(), base_maps->size(), 1, tempAGain.data());
			for ( int k = 0; k < model_weight_.size(); ++k ) grid_model_weight_[temp_grid_index][k] += tempAGain[k];
			MatrixMul(A.data(), model_weight_.size(), base_maps->size(), SIGMA.data(), base_maps->size(), base_maps->size(), tempASigma.data());
			MatrixT(A.data(), model_weight_.size(), base_maps->size(), AT.data());
			MatrixMul(tempASigma.data(), model_weight_.size(), base_maps->size(), AT.data(), base_maps->size(), model_weight_.size(), tempASigmaA.data());
			for ( int k = 0; k < R.size(); ++k ) covariance_vec_[temp_grid_index][k] = R[k] - tempASigmaA[k];

			temp_longitude_index += grid_size_;
			temp_grid_index++;
		}
		temp_latitude_index += grid_size_;
	}*/
}

void WrfDataManager::GenerateBaseMaps(){
	GenerateInitialModelWeight();


	for ( int i = current_time_ - history_time_length_; i < current_time_; ++i ){
		std::vector< WrfDataStamp* >* base_stamp = new std::vector< WrfDataStamp* >;
		std::vector< WrfDataStamp* >* num_stamp = numerical_data_[0]->at(i);
		for ( int j = 0; j < num_stamp->size(); ++j ){
			WrfGridDataStamp* temp_grid_stamp = dynamic_cast< WrfGridDataStamp* >(num_stamp->at(j));
			WrfGridDataStamp* new_grid_stamp = new WrfGridDataStamp(temp_grid_stamp->stamp_type(), temp_grid_stamp->time(), temp_grid_stamp->data_type());
			new_grid_stamp->start_longitude = start_longitude_;
			new_grid_stamp->end_longitude = end_longitude_;
			new_grid_stamp->start_latitude = start_latitude_;
			new_grid_stamp->end_latitude = end_latitude_;
			new_grid_stamp->longitude_grid_space = longitude_grid_space_;
			new_grid_stamp->latitude_grid_space = latitude_grid_space_;
			new_grid_stamp->longitude_grid_number = longitude_grid_number_;
			new_grid_stamp->latitude_grid_number = latitude_grid_number_;
			switch ( new_grid_stamp->data_type() ){
			case WRF_RAIN:
			case WRF_PRESSURE:
			case WRF_TEMPERATURE_850HPA:
			case WRF_HEIGHT_850HPA:
			case WRF_RH_850HPA:
				new_grid_stamp->values.resize(longitude_grid_number_ * latitude_grid_number_, 0);	
				break;
			case WRF_WIND_850HPA:
				new_grid_stamp->values.resize(longitude_grid_number_ * latitude_grid_number_ * 2, 0);
				break;
			default:
				exit(0);
				break;
			}
			base_stamp->push_back(new_grid_stamp);
		}
		for ( int j = 0; j < numerical_data_.size(); ++j ){
			std::vector< WrfDataStamp* >* num_stamp_vec = numerical_data_[j]->at(i);
			for ( int k = 0; k < num_stamp_vec->size(); ++k ){
				WrfGridDataStamp* temp_grid_stamp = dynamic_cast< WrfGridDataStamp* >(base_stamp->at(k));
				WrfGridDataStamp* temp_num_stamp = dynamic_cast< WrfGridDataStamp* >(num_stamp_vec->at(k));
				WrfGridDataStamp* converted_stamp = data_converter_->Convert(temp_num_stamp, temp_grid_stamp);
				for ( int n = 0; n < temp_grid_stamp->values.size(); ++n ) temp_grid_stamp->values[n] += model_weight_[j] * converted_stamp->values[n];
				delete converted_stamp;
			}
		}
		base_map_data_.insert(DataStampMap::value_type(i, base_stamp));
	}
}

void WrfDataManager::GenerateInitialModelWeight(){
	float average_weight = 1.0 / numerical_data_.size();
	model_weight_.resize(numerical_data_.size(), average_weight);

	std::vector< float > bias_sum;
	bias_sum.resize(numerical_data_.size(), 0);
	for ( int i = 0; i < history_time_length_; ++i ){
		std::vector< WrfDataStamp* >* his_data = historical_data_[current_time_ - i];
		for ( int k = 0; k < numerical_data_.size(); ++k ){
			std::vector< WrfDataStamp* >* num_data = numerical_data_[k]->at(current_time_ - i);
			for ( int j = 0; j < his_data->size(); ++j ){
				switch ( his_data->at(j)->format_type() ){
				case WRF_FORMAT_THREE:
					{
						WrfDiscreteDataStamp* temp_stamp = dynamic_cast< WrfDiscreteDataStamp* >(his_data->at(j));
						WrfGridDataStamp* temp_num_stamp = dynamic_cast< WrfGridDataStamp* >(num_data->at(j));
						WrfDiscreteDataStamp* num_interpolated_stamp = data_converter_->Convert(temp_num_stamp, temp_stamp, INTERPOLATION_RADIUS);
						float temp_sum = 0;
						for ( int n = 0; n < num_interpolated_stamp->values.size(); ++n ){
							float temp_value1;
							float temp_value2;
							float temp_value3 = 0;
							temp_value1 = (num_interpolated_stamp->values[n].value2- attribute_min_value_[num_interpolated_stamp->data_type()]) 
								/ (attribute_max_value_[num_interpolated_stamp->data_type()] - attribute_min_value_[num_interpolated_stamp->data_type()]);
							temp_value2 = (temp_stamp->values[n].value2 - attribute_min_value_[temp_stamp->data_type()]) 
								/ (attribute_max_value_[num_interpolated_stamp->data_type()] - attribute_min_value_[temp_stamp->data_type()]);
							temp_value3 += abs(temp_value1 - temp_value2) / (temp_value2 + 0.01);

							switch ( temp_stamp->data_type() ){
							case WRF_RAIN:
							case WRF_PRESSURE:
							case WRF_TEMPERATURE_850HPA:
							case WRF_HEIGHT_850HPA:
							case WRF_RH_850HPA:
								break;
							case WRF_WIND_850HPA:
								temp_value1 = (num_interpolated_stamp->values[n].value1 - attribute_min_value_[num_interpolated_stamp->data_type()]) 
									/ (attribute_max_value_[num_interpolated_stamp->data_type()] - attribute_min_value_[num_interpolated_stamp->data_type()]);
								temp_value2 = (temp_stamp->values[n].value1 - attribute_min_value_[temp_stamp->data_type()]) 
									/ (attribute_max_value_[num_interpolated_stamp->data_type()] - attribute_min_value_[temp_stamp->data_type()]);
								temp_value3 += abs(temp_value1 - temp_value2) / (temp_value2 + 0.01);

								temp_value3 /= 2;
								break;
							default:
								exit(0);
								break;
							}
							temp_sum += temp_value3;
						}

						temp_sum /= num_interpolated_stamp->values.size();
						bias_sum[k] += temp_sum;
						delete num_interpolated_stamp;
						break;
					}
				default:
					exit(0);
					break;
				}
			}
		}
	}

	float total_sum = 0;
	for ( int i = 0; i < bias_sum.size(); ++i ) total_sum += bias_sum[i];
	for ( int i = 0; i < model_weight_.size(); ++i ){
		model_weight_[i] = (total_sum - bias_sum[i]) / ((model_weight_.size() - 1) * total_sum);
	}
}

void WrfDataManager::UpdateHistoricalData(){
	
}

void WrfDataManager::LoadT639(){
	// Load T639
	QString t639_dir = QString::fromLocal8Bit(root_dir_.c_str()) + QString("t639/");
	std::vector< WrfDataStamp* > stamp_vec;

	int record_size = 0;
	// Load Rain24
	QString rain_dir = t639_dir + QString("rain24/");
	LoadGridData(rain_dir, WRF_NUMERICAL_STAMP, WRF_RAIN, stamp_vec, "1304*08.024");
	record_size = stamp_vec.size();
	// Load Sea Level Pressure
	QString pressure_dir = t639_dir + "pressure/";
	LoadGridData(pressure_dir, WRF_NUMERICAL_STAMP, WRF_PRESSURE, stamp_vec, "1304*08.024");
	// Load 850hpa Temperature
	QString temper_850hpa_dir = t639_dir + "temper/850/";
	LoadGridData(temper_850hpa_dir, WRF_NUMERICAL_STAMP, WRF_TEMPERATURE_850HPA, stamp_vec, "1304*08.024");
	// Load 850hpa Wind Speed and Wind Direction
	QString wind_850hpa_dir = t639_dir + "wind/850/";
	LoadHighAltitudeData(wind_850hpa_dir, WRF_NUMERICAL_STAMP, stamp_vec, "1304*08.024");
	// Load 850hpa Height
	QString height_850hpa_dir = t639_dir + "height/850/";
	LoadGridData(height_850hpa_dir, WRF_NUMERICAL_STAMP, WRF_HEIGHT_850HPA, stamp_vec, "1304*08.024");
	// Load 850hpa Relative Humidity
	QString rh_1000hpa_dir = t639_dir + "rh/1000/";
	LoadGridData(rh_1000hpa_dir, WRF_NUMERICAL_STAMP, WRF_RH_850HPA, stamp_vec, "1304*08.024");

	// Convert to data record set
	WrfGridDataStamp* template_stamp = dynamic_cast< WrfGridDataStamp* >(stamp_vec[0]);
	if ( template_stamp == NULL ){
		std::cout << "Error loading template stamp!" << std::endl;
		return;
	}

	DataStampMap* data_map = new DataStampMap;
	for ( int i = 0; i < record_size; ++i ){
		int time_int = stamp_vec[i]->time().ToInt();

		std::vector< WrfDataStamp* >* record_stamps = new std::vector< WrfDataStamp* >;
		record_stamps->push_back(stamp_vec[i]);
		for ( int k  = record_size; k < stamp_vec.size(); ++k ){
			if ( stamp_vec[k]->time().ToInt() == time_int ) record_stamps->push_back(stamp_vec[k]);
		}

		data_map->insert(DataStampMap::value_type(time_int, record_stamps));
	}
	numerical_data_.push_back(data_map);
}

void WrfDataManager::LoadEcFine(){
	// Load EcFine
	QString ecfine_dir = QString::fromLocal8Bit(root_dir_.c_str()) + QString("newecmwf_grib/");
	std::vector< WrfDataStamp* > stamp_vec;

	int record_size = 0;
	// Load Rain24
	QString rain_dir = ecfine_dir + QString("rain24/rain02/");
	LoadGridData(rain_dir, WRF_NUMERICAL_STAMP, WRF_RAIN, stamp_vec, "1304*08.024");
	record_size = stamp_vec.size();
	// Load Sea Level Pressure
	QString pressure_dir = ecfine_dir + "pressure/";
	LoadGridData(pressure_dir, WRF_NUMERICAL_STAMP, WRF_PRESSURE, stamp_vec, "1304*08.024");
	// Load 850hpa Temperature
	QString temper_850hpa_dir = ecfine_dir + "temper/850/";
	LoadGridData(temper_850hpa_dir, WRF_NUMERICAL_STAMP, WRF_TEMPERATURE_850HPA, stamp_vec, "1304*08.024");
	// Load 850hpa Wind Speed and Wind Direction
	QString wind_850hpa_dir = ecfine_dir + "wind/850/";
	LoadHighAltitudeData(wind_850hpa_dir, WRF_NUMERICAL_STAMP, stamp_vec, "1304*08.024");
	// Load 850hpa Height
	QString height_850hpa_dir = ecfine_dir + "height/500/";
	LoadGridData(height_850hpa_dir, WRF_NUMERICAL_STAMP, WRF_HEIGHT_850HPA, stamp_vec, "1304*08.024");
	// Load 850hpa Relative Humidity
	QString rh_1000hpa_dir = ecfine_dir + "rh/1000/";
	LoadGridData(rh_1000hpa_dir, WRF_NUMERICAL_STAMP, WRF_RH_850HPA, stamp_vec, "1304*08.024");

	DataStampMap* data_map = new DataStampMap;
	for ( int i = 0; i < record_size; ++i ){
		int time_int = stamp_vec[i]->time().ToInt();

		std::vector< WrfDataStamp* >* record_stamps = new std::vector< WrfDataStamp* >;
		record_stamps->push_back(stamp_vec[i]);
		for ( int k  = record_size; k < stamp_vec.size(); ++k ){
			if ( stamp_vec[k]->time().ToInt() == time_int ) record_stamps->push_back(stamp_vec[k]);
		}

		data_map->insert(DataStampMap::value_type(time_int, record_stamps));
	}
	numerical_data_.push_back(data_map);
}

void WrfDataManager::LoadNcep(){
	// Load Ncep
	QString ecfine_dir = QString::fromLocal8Bit(root_dir_.c_str()) + QString("NCEP_GFS/");
	std::vector< WrfDataStamp* > stamp_vec;

	int record_size = 0;
	// Load Rain24
	QString rain_dir = ecfine_dir + QString("rain02/");
	LoadGridData(rain_dir, WRF_NUMERICAL_STAMP, WRF_RAIN, stamp_vec, "1304*00.024");
	record_size = stamp_vec.size();
	// Load Sea Level Pressure
	QString pressure_dir = ecfine_dir + "Pmsl/";
	LoadGridData(pressure_dir, WRF_NUMERICAL_STAMP, WRF_PRESSURE, stamp_vec, "1304*00.024");
	// Load 850hpa Temperature
	QString temper_850hpa_dir = ecfine_dir + "T/850/";
	LoadGridData(temper_850hpa_dir, WRF_NUMERICAL_STAMP, WRF_TEMPERATURE_850HPA, stamp_vec, "1304*00.024");
	// Load 850hpa Wind Speed and Wind Direction
	QString wind_850hpa_dir = ecfine_dir + "Wind/850/";
	LoadHighAltitudeData(wind_850hpa_dir, WRF_NUMERICAL_STAMP, stamp_vec, "1304*00.024");
	// Load 850hpa Height
	QString height_850hpa_dir = ecfine_dir + "H/500/";
	LoadGridData(height_850hpa_dir, WRF_NUMERICAL_STAMP, WRF_HEIGHT_850HPA, stamp_vec, "1304*00.024");
	// Load 850hpa Relative Humidity
	QString rh_1000hpa_dir = ecfine_dir + "RH/1000/";
	LoadGridData(rh_1000hpa_dir, WRF_NUMERICAL_STAMP, WRF_RH_850HPA, stamp_vec, "1304*00.024");

	DataStampMap* data_map = new DataStampMap;
	for ( int i = 0; i < record_size; ++i ){
		int time_int = stamp_vec[i]->time().ToInt();

		std::vector< WrfDataStamp* >* record_stamps = new std::vector< WrfDataStamp* >;
		record_stamps->push_back(stamp_vec[i]);
		for ( int k  = record_size; k < stamp_vec.size(); ++k ){
			if ( stamp_vec[k]->time().ToInt() == time_int ) record_stamps->push_back(stamp_vec[k]);
		}

		data_map->insert(DataStampMap::value_type(time_int, record_stamps));
	}
	numerical_data_.push_back(data_map);
}

void WrfDataManager::LoadJapanGsm(){
	// Load Japan-Gsm
	QString ecfine_dir = QString::fromLocal8Bit(root_dir_.c_str()) + QString("japan_gsm/");
	std::vector< WrfDataStamp* > stamp_vec;

	int record_size = 0;
	// Load Rain24
	QString rain_dir = ecfine_dir + QString("rain/rain24/rain02/");
	LoadGridData(rain_dir, WRF_NUMERICAL_STAMP, WRF_RAIN, stamp_vec, "1304*08.024");
	record_size = stamp_vec.size();
	// Load Sea Level Pressure
	QString pressure_dir = ecfine_dir + "pressure/";
	LoadGridData(pressure_dir, WRF_NUMERICAL_STAMP, WRF_PRESSURE, stamp_vec, "1304*08.024");
	// Load 850hpa Temperature
	QString temper_850hpa_dir = ecfine_dir + "temper/850/";
	LoadGridData(temper_850hpa_dir, WRF_NUMERICAL_STAMP, WRF_TEMPERATURE_850HPA, stamp_vec, "1304*08.024");
	// Load 850hpa Wind Speed and Wind Direction
	QString wind_850hpa_dir = ecfine_dir + "wind/850/";
	LoadHighAltitudeData(wind_850hpa_dir, WRF_NUMERICAL_STAMP, stamp_vec, "1304*08.024");
	// Load 850hpa Height
	QString height_850hpa_dir = ecfine_dir + "height/500/";
	LoadGridData(height_850hpa_dir, WRF_NUMERICAL_STAMP, WRF_HEIGHT_850HPA, stamp_vec, "1304*08.024");
	// Load 850hpa Relative Humidity
	QString rh_1000hpa_dir = ecfine_dir + "rh/1000/";
	LoadGridData(rh_1000hpa_dir, WRF_NUMERICAL_STAMP, WRF_RH_850HPA, stamp_vec, "1304*08.024");

	DataStampMap* data_map = new DataStampMap;
	for ( int i = 0; i < record_size; ++i ){
		int time_int = stamp_vec[i]->time().ToInt();

		std::vector< WrfDataStamp* >* record_stamps = new std::vector< WrfDataStamp* >;
		record_stamps->push_back(stamp_vec[i]);
		for ( int k  = record_size; k < stamp_vec.size(); ++k ){
			if ( stamp_vec[k]->time().ToInt() == time_int ) record_stamps->push_back(stamp_vec[k]);
		}

		data_map->insert(DataStampMap::value_type(time_int, record_stamps));
	}
	numerical_data_.push_back(data_map);
}

void WrfDataManager::LoadScene(){
	QString scene_dir = QString::fromLocal8Bit(root_dir_.c_str()) + "surface/";
	std::vector< WrfDataStamp* > stamp_vec;

	int record_size;
	// Load rain24
	QString rain_file_dir = scene_dir + "r24-8/";
	LoadDiscreteData(rain_file_dir, WRF_HISTORICAL_STAMP, WRF_RAIN, stamp_vec, "1304*08.000");
	record_size = stamp_vec.size();

	// Load sea level pressure
	QString pressure_dir = scene_dir + "p0-p/";
	LoadDiscreteData(pressure_dir, WRF_HISTORICAL_STAMP, WRF_PRESSURE, stamp_vec, "1304*08.000");
	for ( int i = record_size; i < stamp_vec.size(); ++i ){
		WrfDiscreteDataStamp* data_stamp = dynamic_cast< WrfDiscreteDataStamp* >(stamp_vec.at(i));
		for ( int k = 0; k < data_stamp->values.size(); ++k ) data_stamp->values[k].value2 += 1000;
	}

	// High altitude data
	QString high_dir = QString::fromLocal8Bit(root_dir_.c_str()) + "high/by/plot/850";
	LoadHighAltitudeData(high_dir, WRF_HISTORICAL_STAMP, stamp_vec, "1304*08.000");

	QString height_dir = QString::fromLocal8Bit(root_dir_.c_str()) + "high/by/plot/500";
	LoadHighAltitudeData(height_dir, WRF_HISTORICAL_STAMP, stamp_vec, "1304*08.000");

	// RH data
	QString rh_dir = scene_dir + "rh/";
	LoadDiscreteData(rh_dir, WRF_HISTORICAL_STAMP, WRF_RH_850HPA, stamp_vec, "1304*06.000");

	for ( int i = 0; i < record_size; ++i ){
		int time_int = stamp_vec[i]->time().ToInt();

		std::vector< WrfDataStamp* >* record_stamps = new std::vector< WrfDataStamp* >;
		record_stamps->push_back(stamp_vec[i]);
		for ( int k  = record_size; k < stamp_vec.size(); ++k )
			if ( stamp_vec[k]->time().ToInt() == time_int ) record_stamps->push_back(stamp_vec[k]);

		historical_data_[time_int] = record_stamps;
	}
}

void WrfDataManager::LoadGridData(QString file_path, WrfDataStampType stamp_type, WrfGeneralDataStampType data_type, std::vector< WrfDataStamp* >& stamp_vec, QString filter){
	QDir file_dir(file_path);
	QStringList filters;
	filters << filter;
	file_dir.setNameFilters(filters);
	QFileInfoList file_list = file_dir.entryInfoList();
	for ( int i = 0; i < file_list.size(); ++i ){
		QString temp_file_name = file_list.at(i).absoluteFilePath();

		/*QString temp_save_name = temp_file_name.right(temp_file_name.length() - 1);
		temp_save_name = QString("E") + temp_save_name;
		QString dir_path = file_list.at(i).absolutePath();
		dir_path = dir_path.right(dir_path.length() - 1);
		dir_path = QString("E") + dir_path;
		QDir dir = QDir("E:");
		if ( !QDir(dir_path).exists()) dir.mkpath(dir_path);
		bool b = QFile::copy(temp_file_name, temp_save_name);*/

		QString temp_str = temp_file_name.right(12);
		temp_str = temp_str.left(8);
		WrfTime time;
		time.year = temp_str.left(2).toInt();
		time.month = temp_str.mid(2, 2).toInt();
		time.day = temp_str.mid(4, 2).toInt();
		time.hour = temp_str.right(2).toInt();
		if ( time.ToInt() > current_time_ || time.ToInt() < current_time_ - history_time_length_ ) continue;
		WrfGridDataStamp* grid_stamp = new WrfGridDataStamp(stamp_type, time, temp_file_name.toLocal8Bit().data(), data_type);
		stamp_vec.push_back(grid_stamp);
	}
}

void WrfDataManager::LoadGridDataFormatThree(QString file_path, WrfDataStampType stamp_type, WrfGeneralDataStampType data_type, std::vector< WrfDataStamp* >& stamp_vec, QString filter){
	QDir file_dir(file_path);
	QStringList filters;
	filters << filter;
	file_dir.setNameFilters(filters);
	QFileInfoList file_list = file_dir.entryInfoList();
	for ( int i = 0; i < file_list.size(); ++i ){
		QString temp_file_name = file_list.at(i).absoluteFilePath();

		/*QString temp_save_name = temp_file_name.right(temp_file_name.length() - 1);
		temp_save_name = QString("E") + temp_save_name;
		QString dir_path = file_list.at(i).absolutePath();
		dir_path = dir_path.right(dir_path.length() - 1);
		dir_path = QString("E") + dir_path;
		QDir dir = QDir("E:");
		if ( !QDir(dir_path).exists()) dir.mkpath(dir_path);
		bool b = QFile::copy(temp_file_name, temp_save_name);*/

		QString temp_str = temp_file_name.right(12);
		temp_str = temp_str.left(8);
		WrfTime time;
		time.year = temp_str.left(2).toInt();
		time.month = temp_str.mid(2, 2).toInt();
		time.day = temp_str.mid(4, 2).toInt();
		time.hour = temp_str.right(2).toInt();
		if ( time.ToInt() > current_time_ || time.ToInt() < current_time_ - history_time_length_ ) continue;
		WrfDiscreteDataStamp* discrete_stamp = new WrfDiscreteDataStamp(stamp_type, time, temp_file_name.toLocal8Bit().data(), data_type);

		WrfGridDataStamp* grid_stamp = data_converter_->ConvertT(discrete_stamp);

		delete discrete_stamp;

		stamp_vec.push_back(grid_stamp);
	}
}

void WrfDataManager::LoadHighAltitudeData(QString file_path, WrfDataStampType stamp_type, std::vector< WrfDataStamp* >& stamp_vec, QString filter){
	QDir file_dir(file_path); 
	QStringList filters;
	filters << filter;
	file_dir.setNameFilters(filters);
	QFileInfoList file_list = file_dir.entryInfoList();
	for ( int i = 0; i < file_list.size(); ++i ){
		QString temp_file_name = file_list.at(i).absoluteFilePath();

		/*QString temp_save_name = temp_file_name.right(temp_file_name.length() - 1);
		temp_save_name = QString("E") + temp_save_name;
		QString dir_path = file_list.at(i).absolutePath();
		dir_path = dir_path.right(dir_path.length() - 1);
		dir_path = QString("E") + dir_path;
		QDir dir = QDir("E:");
		if ( !QDir(dir_path).exists()) dir.mkpath(dir_path);
		bool b = QFile::copy(temp_file_name, temp_save_name);*/

		QString temp_str = temp_file_name.right(12);
		temp_str = temp_str.left(8);
		WrfTime time;
		time.year = temp_str.left(2).toInt();
		time.month = temp_str.mid(2, 2).toInt();
		time.day = temp_str.mid(4, 2).toInt();
		time.hour = temp_str.right(2).toInt();
		if ( time.ToInt() > current_time_ || time.ToInt() < current_time_ - history_time_length_ ) continue;
		WrfHighAltitudeDataStamp* grid_stamp = new WrfHighAltitudeDataStamp(stamp_type, time, temp_file_name.toLocal8Bit().data());
		std::vector< WrfDataStamp* > temp_stamp_vec;
		data_converter_->Convert(grid_stamp, temp_stamp_vec);
		delete grid_stamp;

		QString temp_height = file_path.right(3);
		if ( temp_height.toInt() == 850 && stamp_type == WRF_HISTORICAL_STAMP ){
			for ( int k = 0; k < temp_stamp_vec.size() - 1; ++k ) stamp_vec.push_back(temp_stamp_vec[k]);
			delete temp_stamp_vec[temp_stamp_vec.size() - 1];
		} else if ( temp_height.toInt() == 500 && stamp_type == WRF_HISTORICAL_STAMP ){
			stamp_vec.push_back(temp_stamp_vec[temp_stamp_vec.size() - 1]);
			for ( int k = 0; k < temp_stamp_vec.size() - 1; ++k ) delete temp_stamp_vec[k];
		} else {
			for ( int k = 0; k < temp_stamp_vec.size(); ++k ) stamp_vec.push_back(temp_stamp_vec[k]);
		}
	}
}

void WrfDataManager::LoadDiscreteData(QString file_path, WrfDataStampType stamp_type, WrfGeneralDataStampType data_type, std::vector< WrfDataStamp* >& stamp_vec, QString filter){
	QDir file_dir(file_path);
	QStringList filters;
	filters << filter;
	file_dir.setNameFilters(filters);
	QFileInfoList file_list = file_dir.entryInfoList();
	for ( int i = 0; i < file_list.size(); ++i ){
		QString temp_file_name = file_list.at(i).absoluteFilePath();

		/*QString temp_save_name = temp_file_name.right(temp_file_name.length() - 1);
		temp_save_name = QString("E") + temp_save_name;
		QString dir_path = file_list.at(i).absolutePath();
		dir_path = dir_path.right(dir_path.length() - 1);
		dir_path = QString("E") + dir_path;
		QDir dir = QDir("E:");
		if ( !QDir(dir_path).exists()) dir.mkpath(dir_path);
		bool b = QFile::copy(temp_file_name, temp_save_name);*/

		QString temp_str = temp_file_name.right(12);
		temp_str = temp_str.left(8);
		WrfTime time;
		time.year = temp_str.left(2).toInt();
		time.month = temp_str.mid(2, 2).toInt();
		time.day = temp_str.mid(4, 2).toInt();
		time.hour = temp_str.right(2).toInt();
		if ( time.ToInt() > current_time_ || time.ToInt() < current_time_ - history_time_length_ ) continue;
		WrfDiscreteDataStamp* grid_stamp = new WrfDiscreteDataStamp(stamp_type, time, temp_file_name.toLocal8Bit().data(), data_type);
		stamp_vec.push_back(grid_stamp);
	}
}

WrfDataRecordSet* WrfDataManager::GetNumericalDataRecordSet(const WrfTime& time){
	return NULL;
}

WrfDataRecordSet* WrfDataManager::GetHistoricalDataRecordSet(const WrfTime& time){
	return NULL;
}

void WrfDataManager::set_seleted_grid_index(std::vector< int >& index){
	int longi_grid = longitude_grid_number_ / grid_size_;
	int lati_grid = latitude_grid_number_ / grid_size_;

	selected_patch_index_.resize(longi_grid * lati_grid);
	selected_patch_index_.assign(selected_patch_index_.size(), false);

	selected_grid_index_.resize(longitude_grid_number_ * latitude_grid_number_);
	selected_grid_index_.assign(selected_grid_index_.size(), false);

	for ( int i = 0; i < index.size(); ++i ) {
		int temp_index = index[i];
		selected_grid_index_[temp_index] = true;

		int temp_longi = temp_index % longitude_grid_number_;
		int temp_lati = temp_index / longitude_grid_number_;
		temp_longi /= 3;
		temp_lati /= 3;
		selected_patch_index_[temp_lati * longi_grid + temp_longi] = true;
	}
}

void WrfDataManager::set_compared_grid_index(std::vector< int >& index){
	compared_grid_index_.resize(longitude_grid_number_ * latitude_grid_number_);
	compared_grid_index_.assign(selected_grid_index_.size(), false);

	for ( int i = 0; i < index.size(); ++i ) {
		int temp_index = index[i];
		compared_grid_index_[temp_index] = true;
	}
}

void WrfDataManager::MatrixMul(float* matrix1, int m1, int n1, float* matrix2, int m2, int n2, float* result){
	for ( int i = 0; i < m1; ++i )
		for (int j = 0; j < n2; ++j ){
			float temp = 0;
			for ( int k = 0; k < n1; ++k ) 
				temp += matrix1[i * n1 + k] * matrix2[k * n2 + j];
			result[i * n2 + j] = temp;
		}
}

void WrfDataManager::MatrixT(float* matrix, int m, int n, float* result){
	for ( int i = 0; i < m; ++i )
		for ( int j = 0; j < n; ++j )
			result[j * m + i] = matrix[i * n + j];
}

void WrfDataManager::MatrixInv(float* matrix, int m, int n, float* result){
	if ( m != n ){
		std::cout << "Error invert matrix!!! n != m" << std::endl;
		return;
	}

	std::vector< int > IPIV;
	IPIV.resize(n);

	for ( int i = 0; i < m * n; ++i ) result[i] = matrix[i];

	lapack_int info = 0, lda;
	lda = n;
	info |= LAPACKE_sgetrf(LAPACK_ROW_MAJOR, m, n, matrix, lda, IPIV.data());
	info |= LAPACKE_sgetri(LAPACK_ROW_MAJOR, n, result, lda, IPIV.data());
	if ( info != 0 ){
		std::cout << "Error Invert Matrix, calculation error!" << std::endl;
	}
}

void WrfDataManager::GetAverageInfoGainMaps(std::vector< WrfGridValueMap* >& maps, std::vector< std::vector< float > >& weights, std::vector< int >& changed_index){
	changed_index.clear();
	for ( int i = 0; i < info_gain_maps.size(); ++i ) delete info_gain_maps[i];
	info_gain_weight_vec.clear();
	info_gain_maps.clear();
	info_gain_weight_vec.push_back(attribute_weight_);
	changed_index.push_back(-1);

	float incremental_step = 0.2;
	std::vector< float > temp_attribute_weight;
	for ( int i = 0; i < attribute_weight_.size(); ++i ){
		temp_attribute_weight.assign(attribute_weight_.begin(), attribute_weight_.end());

		float current_weight = temp_attribute_weight[i] + incremental_step;
		if ( current_weight > 1.0 ) current_weight = 1.0;
		float weight_bias = current_weight - temp_attribute_weight[i];
		float sum_all = 0;
		for ( int j = 0; j < temp_attribute_weight.size(); ++j )
			if ( i != j ) sum_all += temp_attribute_weight[j];
		if ( abs(sum_all) > 1e-5 ){
			temp_attribute_weight[i] = current_weight;
			for ( int j = 0; j < temp_attribute_weight.size(); ++j )
				if ( i != j ) temp_attribute_weight[j] -= weight_bias * temp_attribute_weight[j] / sum_all;
			info_gain_weight_vec.push_back(temp_attribute_weight);
			changed_index.push_back(i);
		}

		temp_attribute_weight.assign(attribute_weight_.begin(), attribute_weight_.end());
		if ( abs(temp_attribute_weight[i]) < 1e-5 ) continue;
		current_weight = temp_attribute_weight[i] - incremental_step;
		if ( current_weight < 0 ) current_weight = 0;
		weight_bias = current_weight - temp_attribute_weight[i];
		sum_all = 0;
		for ( int j = 0; j < temp_attribute_weight.size(); ++j )
			if ( i != j ) sum_all += temp_attribute_weight[j];
		if ( abs(sum_all) > 1e-5 ){
			for ( int j = 0; j < temp_attribute_weight.size(); ++j )
				if ( i != j ) temp_attribute_weight[j] -= weight_bias * temp_attribute_weight[j] / sum_all;
		} else {
			for ( int j = 0; j < temp_attribute_weight.size(); ++j )
				if ( i != j ) temp_attribute_weight[j] -= weight_bias * 1.0 / (temp_attribute_weight.size() - 1);
		}
		temp_attribute_weight[i] = current_weight;
		info_gain_weight_vec.push_back(temp_attribute_weight);

		changed_index.push_back(i);
	}

	SystemTimer::GetInstance()->BeginTimer();
	for ( int i = 0; i < info_gain_weight_vec.size(); ++i ){
		WrfGridValueMap* value_map = new WrfGridValueMap;

		value_map->start_latitude = start_latitude_;
		value_map->end_latitude = end_latitude_;
		value_map->start_longitude = start_longitude_;
		value_map->end_longitude = end_longitude_;
		value_map->latitude_grid_number = latitude_grid_number_;
		value_map->longitude_grid_number = longitude_grid_number_;
		value_map->latitude_grid_space = latitude_grid_space_;
		value_map->longitude_grid_space = longitude_grid_space_;
		value_map->values.resize(value_map->longitude_grid_number * value_map->latitude_grid_number);
		memset(value_map->values.data(), 0, value_map->values.size() * sizeof(float));

		statistic_solver_->GetBiasVarMap(bias_set_vec_, info_gain_weight_vec[i], grid_model_weight_map_, grid_size_, site_alpha_, current_time_, history_time_length_, history_weight_, current_weight_, value_map);
		value_map->max_value = 1.0;
		value_map->min_value = 0.0;
		info_gain_maps.push_back(value_map);
	}
	SystemTimer::GetInstance()->EndTimer();
	std::cout << "Time used to get average bias var map:  " ;
	SystemTimer::GetInstance()->PrintTime();
	std::cout << std::endl;

	maps.clear();
	maps.assign(info_gain_maps.begin(), info_gain_maps.end());
	weights.clear();
	weights.assign(info_gain_weight_vec.begin(), info_gain_weight_vec.end());
}

void WrfDataManager::GetSingleAttributeInfoGainMaps(int attrib_index, std::vector< WrfGridValueMap* >& maps, std::vector< std::vector< float > >& weights, std::vector< int >& changed_index){
	changed_index.clear();
	for ( int i = 0; i < info_gain_maps.size(); ++i ) delete info_gain_maps[i];
	info_gain_weight_vec.clear();
	info_gain_maps.clear();
	info_gain_weight_vec.push_back(attribute_weight_);
	changed_index.push_back(-1);

	float incremental_step = 0.2;
	std::vector< float > temp_attribute_weight;
	for ( int i = 0; i <= 5; ++i ){
		temp_attribute_weight.assign(attribute_weight_.begin(), attribute_weight_.end());

		float current_weight = i / 5.0;
		float weight_bias = current_weight - temp_attribute_weight[attrib_index];
		if ( abs(weight_bias) < 1e-4 ) continue;
		float sum_all = 0;
		for ( int j = 0; j < temp_attribute_weight.size(); ++j )
			if ( attrib_index != j ) sum_all += temp_attribute_weight[j];
		if ( abs(sum_all) > 1e-5 ){
			temp_attribute_weight[attrib_index] = current_weight;
			for ( int j = 0; j < temp_attribute_weight.size(); ++j )
				if ( attrib_index != j ) temp_attribute_weight[j] -= weight_bias * temp_attribute_weight[j] / sum_all;
			info_gain_weight_vec.push_back(temp_attribute_weight);
			changed_index.push_back(attrib_index);
		}
	}

	SystemTimer::GetInstance()->BeginTimer();
	for ( int i = 0; i < info_gain_weight_vec.size(); ++i ){
		WrfGridValueMap* value_map = new WrfGridValueMap;

		value_map->start_latitude = start_latitude_;
		value_map->end_latitude = end_latitude_;
		value_map->start_longitude = start_longitude_;
		value_map->end_longitude = end_longitude_;
		value_map->latitude_grid_number = latitude_grid_number_;
		value_map->longitude_grid_number = longitude_grid_number_;
		value_map->latitude_grid_space = latitude_grid_space_;
		value_map->longitude_grid_space = longitude_grid_space_;
		value_map->values.resize(value_map->longitude_grid_number * value_map->latitude_grid_number);
		memset(value_map->values.data(), 0, value_map->values.size() * sizeof(float));

		statistic_solver_->GetBiasVarMap(bias_set_vec_, info_gain_weight_vec[i], grid_model_weight_map_, grid_size_, site_alpha_, current_time_, history_time_length_, history_weight_, current_weight_, value_map);
		value_map->max_value = 1.0;
		value_map->min_value = 0.0;
		info_gain_maps.push_back(value_map);
	}
	SystemTimer::GetInstance()->EndTimer();
	std::cout << "Time used to get average bias var map:  " ;
	SystemTimer::GetInstance()->PrintTime();
	std::cout << std::endl;

	maps.clear();
	maps.assign(info_gain_maps.begin(), info_gain_maps.end());
	weights.clear();
	weights.assign(info_gain_weight_vec.begin(), info_gain_weight_vec.end());
}

void WrfDataManager::GetInfoGainWeight(int index, std::vector< float >& weights){
	weights.clear();
	weights.assign(info_gain_weight_vec[index].begin(), info_gain_weight_vec[index].end());
}

int WrfDataManager::AddStoryStamp(WrfGridValueMap* value_stamp, std::vector< int >& related_ids){
	story_line_maps_.push_back(value_stamp);
	map_related_elements_.push_back(related_ids);
	return story_line_maps_.size() - 1;
}

WrfGridValueMap* WrfDataManager::GetStoryStamp(int index){
	return story_line_maps_.at(index);
}

float WrfDataManager::Ftime(int time_bias){
	return exp(-1.0 * abs(time_bias));
}