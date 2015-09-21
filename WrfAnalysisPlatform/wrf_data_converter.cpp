#include "wrf_data_converter.h"

#include <iostream>

WrfDataConverter::WrfDataConverter(){
	
}

WrfDataConverter::~WrfDataConverter(){

}

WrfDataRecordSet* WrfDataConverter::Convert(const std::vector< WrfDataStamp* >& numerical_stamps, const std::vector< WrfDataStamp* >& historical_stamps, 
	const std::vector< WrfDataStamp* >& base_stamps, WrfDataRecordSetType type){

	WrfDataRecordSet* record_set = new WrfDataRecordSet(type);
	record_set->values.resize(longitude_grid_number_ * latitude_grid_number_);
	record_set->his_values.resize(longitude_grid_number_ * latitude_grid_number_);
	for ( int i = 0; i < longitude_grid_number_ * latitude_grid_number_; ++i ) {
		record_set->values[i] = new WrfDataRecord();
		record_set->his_values[i] = new WrfDataRecord();
	}

	for ( int i = 0; i < numerical_stamps.size(); ++i ){
		WrfDataStamp* stamp = numerical_stamps.at(i);
		WrfGridDataStamp* num_grid_stamp = NULL;
		switch ( stamp->format_type() ){
		case WRF_FORMAT_FOUR:
			{
				WrfGridDataStamp* temp_stamp = dynamic_cast< WrfGridDataStamp* >(stamp);
				num_grid_stamp = Convert(temp_stamp);
				break;
			}
		default:
			break;
		}
		for ( int k = 0; k < num_grid_stamp->longitude_grid_number * num_grid_stamp->latitude_grid_number; ++k ){
			switch ( num_grid_stamp->data_type() ){
			case WRF_RAIN:
				record_set->values[k]->rain = num_grid_stamp->values[k];
				record_set->his_values[k]->rain = 0;
				break;
			case WRF_PRESSURE:
				record_set->values[k]->pressure = num_grid_stamp->values[k];
				record_set->his_values[k]->pressure = 0;
				break;
			case WRF_RH_850HPA:
				record_set->values[k]->relative_humidity_850hpa = num_grid_stamp->values[k];
				record_set->his_values[k]->relative_humidity_850hpa = 0;
				break;
			case WRF_HEIGHT_850HPA:
				record_set->values[k]->height_850hpa = num_grid_stamp->values[k];
				record_set->his_values[k]->height_850hpa = 0;
				break;
			case WRF_TEMPERATURE_850HPA:
				record_set->values[k]->temperature_850hpa = num_grid_stamp->values[k];
				record_set->his_values[k]->temperature_850hpa = 0;
				break;
			case WRF_WIND_850HPA:
				record_set->values[k]->x_speed_850hpa = num_grid_stamp->values[2 * k];
				record_set->values[k]->y_speed_850hpa = num_grid_stamp->values[2 * k + 1];
				record_set->his_values[k]->x_speed_850hpa = 0;
				record_set->his_values[k]->y_speed_850hpa = 0;
				break;
			default:
				break;
			}
		}

		delete num_grid_stamp;
	}

	for ( int i = 0; i < historical_stamps.size(); ++i ){
		WrfDataStamp* stamp = historical_stamps.at(i);
		WrfGridDataStamp* base_stamp = dynamic_cast< WrfGridDataStamp* >(base_stamps.at(i));
		WrfGridDataStamp* interpolated_grid_stamp = NULL;
		switch ( stamp->format_type() ){
		case WRF_FORMAT_THREE:
			{
				WrfDiscreteDataStamp* temp_stamp = dynamic_cast< WrfDiscreteDataStamp* >(stamp);
				interpolated_grid_stamp = Interpolate(temp_stamp, base_stamp, 0.05);
				break;
			}
		case WRF_FORMAT_FOUR:
			{
				WrfGridDataStamp* temp_stamp = dynamic_cast< WrfGridDataStamp* >(stamp);
				interpolated_grid_stamp = Convert(temp_stamp);
				break;
			}
		default:
			break;
		}
		for ( int k = 0; k < interpolated_grid_stamp->longitude_grid_number * interpolated_grid_stamp->latitude_grid_number; ++k ){
			switch ( interpolated_grid_stamp->data_type() ){
			case WRF_RAIN:
				record_set->values[k]->rain -= interpolated_grid_stamp->values[k];
				record_set->his_values[k]->rain = interpolated_grid_stamp->values[k];
				break;
			case WRF_PRESSURE:
				record_set->values[k]->pressure -= interpolated_grid_stamp->values[k];
				record_set->his_values[k]->pressure = interpolated_grid_stamp->values[k];
				break;
			case WRF_HEIGHT_850HPA:
				record_set->values[k]->height_850hpa -= interpolated_grid_stamp->values[k];
				record_set->his_values[k]->height_850hpa = interpolated_grid_stamp->values[k];
				break;
			case WRF_TEMPERATURE_850HPA:
				record_set->values[k]->temperature_850hpa -= interpolated_grid_stamp->values[k];
				record_set->his_values[k]->temperature_850hpa = interpolated_grid_stamp->values[k];
				break;
			case WRF_RH_850HPA:
				record_set->values[k]->relative_humidity_850hpa -= base_stamp->values[k];
				record_set->his_values[k]->relative_humidity_850hpa = base_stamp->values[k];
				break;
			case WRF_WIND_850HPA:
				record_set->values[k]->x_speed_850hpa -= interpolated_grid_stamp->values[2 * k];
				record_set->his_values[k]->x_speed_850hpa = interpolated_grid_stamp->values[2 * k];
				record_set->values[k]->y_speed_850hpa -= interpolated_grid_stamp->values[2 * k + 1];
				record_set->his_values[k]->y_speed_850hpa = interpolated_grid_stamp->values[2 * k + 1];
				break;
			default:
				break;
			}
		}

		delete interpolated_grid_stamp;
	}

	return record_set;
}

WrfGridDataStamp* WrfDataConverter::Interpolate(WrfDiscreteDataStamp* discrete_stamp, WrfGridDataStamp* base_stamp, float stop_value){
	WrfGridDataStamp* new_grid_stamp = new WrfGridDataStamp(discrete_stamp->stamp_type(), discrete_stamp->time(), discrete_stamp->data_type());
	new_grid_stamp->start_longitude = start_longitude_;
	new_grid_stamp->end_longitude = end_longitude_;
	new_grid_stamp->start_latitude = start_latitude_;
	new_grid_stamp->end_latitude = end_latitude_;
	new_grid_stamp->longitude_grid_space = longitude_grid_space_;
	new_grid_stamp->latitude_grid_space = latitude_grid_space_;
	new_grid_stamp->longitude_grid_number = longitude_grid_number_;
	new_grid_stamp->latitude_grid_number = latitude_grid_number_;
	new_grid_stamp->values.resize(base_stamp->values.size());
	new_grid_stamp->values.assign(base_stamp->values.begin(), base_stamp->values.end());

	// Step Five: Decrease the radius to run step one to step four, until the increment is smaller than stop value or radius is smaller than 1

	// Step One: Interpolate on the base stamp to get the site value
	WrfDiscreteDataStamp* inter_dis_stamp = Convert(new_grid_stamp, discrete_stamp, INTERPOLATION_RADIUS);
	// Step Two: Calculate the bias value of the site value and interpolated site value
	for ( int i = 0; i < inter_dis_stamp->values.size(); ++i )
		inter_dis_stamp->values[i].value2 = discrete_stamp->values[i].value2 - inter_dis_stamp->values[i].value2;
	// Step Three: Adjust the grid increment based on the accumulation average of the nearest sites in a specified radius
	WrfGridDataStamp* increment_stamp = Convert(inter_dis_stamp, INTERPOLATION_RADIUS);
	// Step Four: Adjust the base map
	for ( int i = 0; i < new_grid_stamp->values.size(); ++i ) {
		new_grid_stamp->values[i] += increment_stamp->values[i];
	}
	
	delete inter_dis_stamp;

	delete increment_stamp;

	return new_grid_stamp;
}

WrfGridDataStamp* WrfDataConverter::Convert(WrfGridDataStamp* grid_stamp, WrfGridDataStamp* template_stamp){
	WrfGridDataStamp* new_grid_stamp = new WrfGridDataStamp(grid_stamp->stamp_type(), grid_stamp->time(), grid_stamp->data_type());
	new_grid_stamp->start_longitude = template_stamp->start_longitude;
	new_grid_stamp->end_longitude = template_stamp->end_longitude;
	new_grid_stamp->start_latitude = template_stamp->start_latitude;
	new_grid_stamp->end_latitude = template_stamp->end_latitude;
	new_grid_stamp->longitude_grid_space = template_stamp->longitude_grid_space;
	new_grid_stamp->latitude_grid_space = template_stamp->latitude_grid_space;
	new_grid_stamp->longitude_grid_number = template_stamp->longitude_grid_number;
	new_grid_stamp->latitude_grid_number = template_stamp->latitude_grid_number;
	new_grid_stamp->values.resize(template_stamp->values.size());

	if ( template_stamp->start_latitude == grid_stamp->start_latitude && template_stamp->end_latitude == grid_stamp->end_latitude 
		&& template_stamp->start_longitude == grid_stamp->start_longitude && template_stamp->end_longitude == grid_stamp->end_longitude
		&& template_stamp->longitude_grid_number == grid_stamp->longitude_grid_number && template_stamp->latitude_grid_number == grid_stamp->latitude_grid_number ){
			new_grid_stamp->values.assign(grid_stamp->values.begin(), grid_stamp->values.end());
	} else {
		for ( int i = 0; i < template_stamp->latitude_grid_number; ++i )
			for ( int j = 0; j < template_stamp->longitude_grid_number; ++j ){
				float longitude = template_stamp->start_longitude + template_stamp->longitude_grid_space * j;
				float latitude = template_stamp->start_latitude + template_stamp->latitude_grid_space * i;
				switch ( grid_stamp->data_type() ){
				case WRF_RAIN:
				case WRF_PRESSURE:
				case WRF_TEMPERATURE_850HPA:
				case WRF_HEIGHT_850HPA:
				case WRF_RH_850HPA:
					new_grid_stamp->values[i * template_stamp->longitude_grid_number + j] = InterpolateScalar(grid_stamp, longitude, latitude, INTERPOLATION_RADIUS);
					break;
				case WRF_WIND_850HPA:
					InterpolateVector2D(grid_stamp, longitude, latitude, INTERPOLATION_RADIUS, 
						new_grid_stamp->values[2 * (i * template_stamp->longitude_grid_number + j)], 
						new_grid_stamp->values[2 * (i * template_stamp->longitude_grid_number + j) + 1]);
					break;
				default:
					exit(0);
					break;
				}
			}
	}

	return new_grid_stamp;
}

WrfDiscreteDataStamp* WrfDataConverter::Convert(WrfGridDataStamp* grid_stamp, WrfDiscreteDataStamp* template_stamp, float radius){
	WrfDiscreteDataStamp* new_stamp = new WrfDiscreteDataStamp(grid_stamp->stamp_type(), grid_stamp->time(), grid_stamp->data_type());
	new_stamp->values.resize(template_stamp->values.size());
	for ( int i = 0; i < template_stamp->values.size(); ++i ){
		DiscreteDataRecord record = template_stamp->values.at(i);
		switch ( grid_stamp->data_type() ){
		case WRF_RAIN:
		case WRF_PRESSURE:
		case WRF_TEMPERATURE_850HPA:
		case WRF_HEIGHT_850HPA:
		case WRF_RH_850HPA:
			record.value2 = InterpolateScalar(grid_stamp, record.longitude, record.latitude, radius);
			break;
		case WRF_WIND_850HPA:
			InterpolateVector2D(grid_stamp, record.longitude, record.latitude, radius, record.value1, record.value2);
			break;
		default:
			exit(0);
			break;
		}
		new_stamp->values[i] = record;
	}

	return new_stamp;
}

WrfGridDataStamp* WrfDataConverter::Convert(WrfDiscreteDataStamp* discrete_stamp, float radius){
	WrfGridDataStamp* new_grid_stamp = new WrfGridDataStamp(discrete_stamp->stamp_type(), discrete_stamp->time(), discrete_stamp->data_type());
	new_grid_stamp->start_longitude = start_longitude_;
	new_grid_stamp->end_longitude = end_longitude_;
	new_grid_stamp->start_latitude = start_latitude_;
	new_grid_stamp->end_latitude = end_latitude_;
	new_grid_stamp->longitude_grid_space = longitude_grid_space_;
	new_grid_stamp->latitude_grid_space = latitude_grid_space_;
	new_grid_stamp->longitude_grid_number = longitude_grid_number_;
	new_grid_stamp->latitude_grid_number = latitude_grid_number_;

	switch ( discrete_stamp->data_type() ){
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
	
	
	for ( int i = 0; i < latitude_grid_number_; ++i )
		for ( int j = 0; j < longitude_grid_number_; ++j ){
			float longitude = start_longitude_ + longitude_grid_space_ * j;
			float latitude = start_latitude_ + latitude_grid_space_ * i;
			
			float value1 = 0, value2 = 0;
			float weight = 0;
			for ( int k = 0; k < discrete_stamp->values.size(); ++k ){
				float temp_dis = Dis(longitude, latitude, discrete_stamp->values[k].longitude, discrete_stamp->values[k].latitude);
				if ( temp_dis < radius){
					float temp_weight = (pow(radius, 2) - pow(temp_dis, 2)) / (pow(radius, 2) + pow(temp_dis, 2));
					switch ( discrete_stamp->data_type() ){
					case WRF_RAIN:
					case WRF_PRESSURE:
					case WRF_TEMPERATURE_850HPA:
					case WRF_HEIGHT_850HPA:
					case WRF_RH_850HPA:
						value2 += discrete_stamp->values[k].value2 * temp_weight;
						break;
					case WRF_WIND_850HPA:
						value1 += discrete_stamp->values[k].value1 * temp_weight;
						value2 += discrete_stamp->values[k].value2 * temp_weight;
						break;
					default:
						exit(0);
						break;
					}
					
					weight += temp_weight;
				}
			}
			if ( weight != 0 ) {
				value1 /= weight;
				value2 /= weight;
			}

			switch ( discrete_stamp->data_type() ){
			case WRF_RAIN:
			case WRF_PRESSURE:
			case WRF_TEMPERATURE_850HPA:
			case WRF_HEIGHT_850HPA:
			case WRF_RH_850HPA:
				new_grid_stamp->values[i * longitude_grid_number_ + j] = value2;
				break;
			case WRF_WIND_850HPA:
				new_grid_stamp->values[2 * (i * longitude_grid_number_ + j)] = value1;
				new_grid_stamp->values[2 * (i * longitude_grid_number_ + j) + 1] = value2;
				break;
			default:
				exit(0);
				break;
			}
			
		}

	return new_grid_stamp;
}

WrfGridDataStamp* WrfDataConverter::Convert(WrfGridDataStamp* grid_stamp){
	WrfGridDataStamp* new_grid_stamp = new WrfGridDataStamp(grid_stamp->stamp_type(), grid_stamp->time(), grid_stamp->data_type());
	new_grid_stamp->start_longitude = start_longitude_;
	new_grid_stamp->end_longitude = end_longitude_;
	new_grid_stamp->start_latitude = start_latitude_;
	new_grid_stamp->end_latitude = end_latitude_;
	new_grid_stamp->longitude_grid_space = longitude_grid_space_;
	new_grid_stamp->latitude_grid_space = latitude_grid_space_;
	new_grid_stamp->longitude_grid_number = longitude_grid_number_;
	new_grid_stamp->latitude_grid_number = latitude_grid_number_;
	switch ( grid_stamp->data_type() ){
	case WRF_RAIN:
	case WRF_PRESSURE:
	case WRF_TEMPERATURE_850HPA:
	case WRF_HEIGHT_850HPA:
	case WRF_RH_850HPA:
		new_grid_stamp->values.resize(latitude_grid_number_ * longitude_grid_number_);
		break;
	case WRF_WIND_850HPA:
		new_grid_stamp->values.resize(latitude_grid_number_ * longitude_grid_number_ * 2);
		break;
	default:
		exit(0);
		break;
	}
	

	if ( start_latitude_ == grid_stamp->start_latitude && end_latitude_ == grid_stamp->end_latitude 
		&& start_longitude_ == grid_stamp->start_longitude && end_longitude_ == grid_stamp->end_longitude
		&& longitude_grid_number_ == grid_stamp->longitude_grid_number && latitude_grid_number_ == grid_stamp->latitude_grid_number ){
		new_grid_stamp->values.assign(grid_stamp->values.begin(), grid_stamp->values.end());
	} else {
		for ( int i = 0; i < latitude_grid_number_; ++i )
			for ( int j = 0; j < longitude_grid_number_; ++j ){
				float longitude = start_longitude_ + longitude_grid_space_ * j;
				float latitude = start_latitude_ + latitude_grid_space_ * i;
				switch ( grid_stamp->data_type() ){
				case WRF_RAIN:
				case WRF_PRESSURE:
				case WRF_TEMPERATURE_850HPA:
				case WRF_HEIGHT_850HPA:
				case WRF_RH_850HPA:
					new_grid_stamp->values[i * longitude_grid_number_ + j] = InterpolateScalar(grid_stamp, longitude, latitude, INTERPOLATION_RADIUS);
					break;
				case WRF_WIND_850HPA:
					InterpolateVector2D(grid_stamp, longitude, latitude, INTERPOLATION_RADIUS, 
						new_grid_stamp->values[2 * (i * longitude_grid_number_ + j)], 
						new_grid_stamp->values[2 * (i * longitude_grid_number_ + j) + 1]);
					break;
				default:
					exit(0);
					break;
				}
				
			}
	}
	
	return new_grid_stamp;
}

void WrfDataConverter::Convert(WrfHighAltitudeDataStamp* high_stamp, std::vector< WrfDataStamp* >& data_stamps){
	HighAltitudeRecord test_record1, test_record2;
	test_record1 = high_stamp->values[0];
	test_record2 = high_stamp->values[1];

	if ( test_record1.latitude != test_record2.latitude ){
		WrfDiscreteDataStamp* temper_stamp = new WrfDiscreteDataStamp(high_stamp->stamp_type(), high_stamp->time(), WRF_TEMPERATURE_850HPA);
		temper_stamp->values.resize(high_stamp->values.size());
		for ( int i = 0; i < high_stamp->values.size(); ++i ){
			DiscreteDataRecord record;
			record.site_id = high_stamp->values[i].site_id;
			record.longitude = high_stamp->values[i].longitude;
			record.latitude = high_stamp->values[i].latitude;
			record.altitude = high_stamp->values[i].altitude;
			record.value2 = high_stamp->values[i].temper;

			temper_stamp->values[i] = record;
		}
		data_stamps.push_back(temper_stamp);

		WrfDiscreteDataStamp* wind_stamp = new WrfDiscreteDataStamp(high_stamp->stamp_type(), high_stamp->time(), WRF_WIND_850HPA);
		wind_stamp->values.resize(high_stamp->values.size());
		for ( int i = 0; i < high_stamp->values.size(); ++i ){
			DiscreteDataRecord record;
			record.site_id = high_stamp->values[i].site_id;
			record.longitude = high_stamp->values[i].longitude;
			record.latitude = high_stamp->values[i].latitude;
			record.altitude = high_stamp->values[i].altitude;
			record.value1 = high_stamp->values[i].x_speed;
			record.value2 = high_stamp->values[i].y_speed;

			wind_stamp->values[i] = record;
		}
		data_stamps.push_back(wind_stamp);

		WrfDiscreteDataStamp* height_stamp = new WrfDiscreteDataStamp(high_stamp->stamp_type(), high_stamp->time(), WRF_HEIGHT_850HPA);
		height_stamp->values.resize(high_stamp->values.size());
		for ( int i = 0; i < high_stamp->values.size(); ++i ){
			DiscreteDataRecord record;
			record.site_id = high_stamp->values[i].site_id;
			record.longitude = high_stamp->values[i].longitude;
			record.latitude = high_stamp->values[i].latitude;
			record.altitude = high_stamp->values[i].altitude;
			record.value2 = high_stamp->values[i].height;

			height_stamp->values[i] = record;
		}
		data_stamps.push_back(height_stamp);
	} else {
		float start_longitude = high_stamp->values[0].longitude;
		float start_latitude = high_stamp->values[0].latitude;
		float end_longitude = high_stamp->values[high_stamp->values.size() - 1].longitude;
		float end_latitude = high_stamp->values[high_stamp->values.size() - 1].latitude;
		float longtitude_space = high_stamp->values[1].longitude - high_stamp->values[0].longitude;
		int temp_index = 1;
		while ( high_stamp->values[temp_index].latitude == high_stamp->values[temp_index - 1].latitude ) ++temp_index;
		float latitude_space = high_stamp->values[temp_index].latitude - high_stamp->values[temp_index - 1].latitude;
		int longitude_grid_number = (int)((end_longitude - start_longitude) / longtitude_space + 0.5) + 1;
		int latitude_grid_number = (int)((end_latitude - start_latitude) / latitude_space + 0.5) + 1;

		if ( longitude_grid_number * latitude_grid_number != high_stamp->values.size() ){
			std::cout << "Error site number!" << std::endl;
			return;
		}

		WrfGridDataStamp* wind_stamp = new WrfGridDataStamp(high_stamp->stamp_type(), high_stamp->time(), WRF_WIND_850HPA);
		wind_stamp->start_longitude = start_longitude;
		wind_stamp->end_longitude = end_longitude;
		wind_stamp->start_latitude = start_latitude;
		wind_stamp->end_latitude = end_latitude;
		wind_stamp->latitude_grid_space = latitude_space;
		wind_stamp->longitude_grid_space = longtitude_space;
		wind_stamp->longitude_grid_number = longitude_grid_number;
		wind_stamp->latitude_grid_number = latitude_grid_number;
		wind_stamp->values.resize(high_stamp->values.size() * 2);
		for ( int i = 0; i < high_stamp->values.size(); ++i ) {
			wind_stamp->values[2 * i] = high_stamp->values[i].x_speed;
			wind_stamp->values[2 * i + 1] = high_stamp->values[i].y_speed;
		}

		data_stamps.push_back(wind_stamp);
	}
}

float WrfDataConverter::Dis(float x1, float y1, float x2, float y2){
	return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

WrfGridDataStamp* WrfDataConverter::ConvertT(WrfDiscreteDataStamp* discrete_stamp){
	float start_longitude = discrete_stamp->values[0].longitude;
	float start_latitude = discrete_stamp->values[0].latitude;
	float end_longitude = discrete_stamp->values[discrete_stamp->values.size() - 1].longitude;
	float end_latitude = discrete_stamp->values[discrete_stamp->values.size() - 1].latitude;
	float longtitude_space = discrete_stamp->values[1].longitude - discrete_stamp->values[0].longitude;
	int temp_index = 1;
	while ( discrete_stamp->values[temp_index].latitude == discrete_stamp->values[temp_index - 1].latitude ) ++temp_index;
	float latitude_space = discrete_stamp->values[temp_index].latitude - discrete_stamp->values[temp_index - 1].latitude;
	int longitude_grid_number = (int)((end_longitude - start_longitude) / longtitude_space + 0.5) + 1;
	int latitude_grid_number = (int)((end_latitude - start_latitude) / latitude_space + 0.5) + 1;

	if ( longitude_grid_number * latitude_grid_number != discrete_stamp->values.size() ){
		std::cout << "Error site number!" << std::endl;
		return NULL;
	}

	WrfGridDataStamp* grid_stamp = new WrfGridDataStamp(discrete_stamp->stamp_type(), discrete_stamp->time(), discrete_stamp->data_type());
	grid_stamp->start_longitude = start_longitude;
	grid_stamp->end_longitude = end_longitude;
	grid_stamp->start_latitude = start_latitude;
	grid_stamp->end_latitude = end_latitude;
	grid_stamp->latitude_grid_space = latitude_space;
	grid_stamp->longitude_grid_space = longtitude_space;
	grid_stamp->longitude_grid_number = longitude_grid_number;
	grid_stamp->latitude_grid_number = latitude_grid_number;
	switch ( grid_stamp->data_type() ){
	case WRF_RAIN:
	case WRF_PRESSURE:
	case WRF_TEMPERATURE_850HPA:
	case WRF_HEIGHT_850HPA:
	case WRF_RH_850HPA:
		grid_stamp->values.resize(discrete_stamp->values.size());
		for ( int i = 0; i < discrete_stamp->values.size(); ++i ) grid_stamp->values[i] = discrete_stamp->values[i].value2;
		break;
	case WRF_WIND_850HPA:
		grid_stamp->values.resize(discrete_stamp->values.size() * 2);
		for ( int i = 0; i < discrete_stamp->values.size(); ++i ) {
			grid_stamp->values[2 * i] = discrete_stamp->values[i].value1;
			grid_stamp->values[2 * i + 1] = discrete_stamp->values[i].value2;
		}
		break;
	default:
		exit(0);
		break;
	}
	
	

	return grid_stamp;
}

float WrfDataConverter::InterpolateScalar(WrfGridDataStamp* grid_stamp, float longitude, float latitude, float radius){
	int negative_indicator = grid_stamp->start_latitude - grid_stamp->end_latitude < 0 ? 1 : -1;
	int long_index = (int)((longitude - radius - grid_stamp->start_longitude) / grid_stamp->longitude_grid_space);
	int lati_index = (int)((latitude - radius * negative_indicator - grid_stamp->start_latitude) / grid_stamp->latitude_grid_space);

	int long_next_index = (int)((longitude + radius - grid_stamp->start_longitude) / grid_stamp->longitude_grid_space);
	int lati_next_index = (int)((latitude + radius * negative_indicator - grid_stamp->start_latitude) / grid_stamp->latitude_grid_space);
	if ( long_index < 0 ) {
		long_index = 0;
		if ( lati_index < 0 ) lati_index = 0;
		if ( lati_index > grid_stamp->latitude_grid_number - 1 ) lati_index = grid_stamp->latitude_grid_number - 1;
		return grid_stamp->values[lati_index * grid_stamp->longitude_grid_number + long_index];
	}
	if ( long_index >= grid_stamp->longitude_grid_number - 1) {
		long_index = grid_stamp->longitude_grid_number - 1;
		if ( lati_index < 0 ) lati_index = 0;
		if ( lati_index > grid_stamp->latitude_grid_number - 1 ) lati_index = grid_stamp->latitude_grid_number - 1;
		return grid_stamp->values[lati_index * grid_stamp->longitude_grid_number + long_index];
	}
	if ( long_next_index >= grid_stamp->longitude_grid_number - 1 ) long_next_index = grid_stamp->longitude_grid_number - 1;
	if ( lati_index < 0 ) {
		lati_index = 0;
		return grid_stamp->values[lati_index * grid_stamp->longitude_grid_number + long_index];
	}
	if ( lati_index >= grid_stamp->latitude_grid_number - 1) {
		lati_index = grid_stamp->latitude_grid_number - 1;
		return grid_stamp->values[lati_index * grid_stamp->longitude_grid_number + long_index];
	}
	if ( lati_next_index >= grid_stamp->latitude_grid_number - 1) lati_next_index = grid_stamp->latitude_grid_number - 1;

	float weight = 0;
	float value = 0;
	for ( int i = lati_index; i < lati_next_index; ++i )
		for ( int j = long_index; j < long_next_index; ++j ){
			float temp_dis = Dis(j * grid_stamp->longitude_grid_space + grid_stamp->start_longitude,
				i * grid_stamp->latitude_grid_space + grid_stamp->start_latitude,
				longitude, latitude);
			if ( temp_dis > radius ) continue;
			float temp_weight = (pow(radius, 2) - pow(temp_dis, 2)) / (pow(radius, 2) + pow(temp_dis, 2));
			value += grid_stamp->values[i * grid_stamp->longitude_grid_number + j] * temp_weight;
			weight += temp_weight;
		}
	if ( weight != 0 ) value /= weight;

	return value;
}

void WrfDataConverter::InterpolateVector2D(WrfGridDataStamp* grid_stamp, float longitude, float latitude, float radius, float& x, float& y){
	int negative_indicator = grid_stamp->start_latitude - grid_stamp->end_latitude < 0 ? 1 : -1;
	int long_index = (int)((longitude - grid_stamp->start_longitude) / grid_stamp->longitude_grid_space);
	int lati_index = (int)((latitude - grid_stamp->start_latitude) / grid_stamp->latitude_grid_space);

	int long_next_index = long_index + 1;
	int lati_next_index = lati_index + negative_indicator;
	if ( long_index < 0 ) long_index = 0;
	if ( long_index >= grid_stamp->longitude_grid_number - 1) long_index = grid_stamp->longitude_grid_number - 1;
	if ( long_next_index < 0 ) long_next_index = 0;
	if ( long_next_index >= grid_stamp->longitude_grid_number - 1 ) long_next_index = grid_stamp->longitude_grid_number - 1;
	if ( lati_index < 0 ) lati_index = 0;
	if ( lati_index >= grid_stamp->latitude_grid_number - 1) lati_index = grid_stamp->latitude_grid_number - 1;
	if ( lati_next_index < 0 ) lati_next_index = 0;
	if ( lati_next_index >= grid_stamp->latitude_grid_number - 1) lati_next_index = grid_stamp->latitude_grid_number - 1;

	float x_scale = (longitude - grid_stamp->start_longitude - long_index * grid_stamp->longitude_grid_space) / grid_stamp->longitude_grid_space;
	if ( abs(x_scale) > 1 ) x_scale = 1;
	float y_scale = (latitude - grid_stamp->start_latitude - lati_index * grid_stamp->latitude_grid_space) / grid_stamp->latitude_grid_space;
	if ( abs(y_scale) > 1 ) y_scale = 1;

	int left_bottom = lati_index * grid_stamp->longitude_grid_number + long_index;
	int left_up = lati_next_index * grid_stamp->longitude_grid_number + long_index;
	int right_bottom = lati_index * grid_stamp->longitude_grid_number + long_next_index;
	int right_up = lati_next_index * grid_stamp->longitude_grid_number + long_next_index;

	float temp_x[2], temp_y[2];
	temp_x[0] = grid_stamp->values[left_bottom * 2] * (1.0 - x_scale) + grid_stamp->values[right_bottom * 2] * x_scale;
	temp_y[0] = grid_stamp->values[left_bottom * 2 + 1] * (1.0 - x_scale) + grid_stamp->values[right_bottom * 2 + 1] * x_scale;

	temp_x[1] = grid_stamp->values[left_up * 2] * (1.0 - x_scale) + grid_stamp->values[right_up * 2] * x_scale;
	temp_y[1] = grid_stamp->values[left_up * 2 + 1] * (1.0 - x_scale) + grid_stamp->values[right_up * 2 + 1] * x_scale;

	x = temp_x[0] * (1.0 - y_scale) + temp_x[1] * y_scale;
	y = temp_y[0] * (1.0 - y_scale) + temp_y[1] * y_scale;
}