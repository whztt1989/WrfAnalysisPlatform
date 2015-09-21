#include "wrf_data_stamp.h"
#include <fstream>

BOOST_CLASS_EXPORT_GUID(WrfDiscreteDataStamp, "WrfDiscreteDataStamp")
BOOST_CLASS_EXPORT_GUID(WrfHighAltitudeDataStamp, "WrfHighAltitudeDataStamp")
BOOST_CLASS_EXPORT_GUID(WrfGridDataStamp, "WrfGridDataStamp")

WrfSurfaceDataStamp::WrfSurfaceDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, const char* file_name)
	: WrfDataStamp(stamp_type_t, time) {
	format_type_ = WRF_FORMAT_ONE;

	if ( !LoadData(file_name) ){
		stamp_type_ = WRF_ERROR_STAMP;
	}
}

WrfSurfaceDataStamp::~WrfSurfaceDataStamp(){

}

bool WrfSurfaceDataStamp::LoadData(const char* file_name){
	return false;
}

WrfHighAltitudeDataStamp::WrfHighAltitudeDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, const char* file_name)
	: WrfDataStamp(stamp_type_t, time){
    format_type_ = WRF_FORMAT_TWO;
	values.clear();

	if ( !LoadData(file_name) ){
		stamp_type_ = WRF_ERROR_STAMP;
	}
}

WrfHighAltitudeDataStamp::~WrfHighAltitudeDataStamp(){

}

bool WrfHighAltitudeDataStamp::LoadData(const char* file_name){
	std::ifstream input(file_name);

	if ( input.good() ){
		getline(input, header_info_);

		int temp_year, temp_month, temp_day, temp_hour, temp_level, site_number;
		input >> temp_year >> temp_month >> temp_day >> temp_hour >> temp_level >> site_number;

		for ( int i = 0; i < site_number; ++i ){
			HighAltitudeRecord record;
			int site_level;
			float theta, speed;
			input >> record.site_id >> record.longitude >> record.latitude >> record.altitude >> site_level;
			input >> record.height >> record.temper >> record.dew_point >> theta >> speed;

			if ( theta == 9999 || speed == 9999 ) continue;

			record.x_speed = cos(theta / 180 * PIE) * speed;
			record.y_speed = sin(theta / 180 * PIE) * speed;

			values.push_back(record);
		}

		input.close();
	} else {
		return false;
	}
	return true;
}

WrfDiscreteDataStamp::WrfDiscreteDataStamp(){

}

WrfDiscreteDataStamp::WrfDiscreteDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, const char* file_name, WrfGeneralDataStampType data_type_t)
	: WrfDataStamp(stamp_type_t, time){
	format_type_ = WRF_FORMAT_THREE;
	data_type_ = data_type_t;
	values.clear();

	if ( !LoadData(file_name) ){
		stamp_type_ = WRF_ERROR_STAMP;
	}
}

WrfDiscreteDataStamp::WrfDiscreteDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, WrfGeneralDataStampType data_type_t)
	: WrfDataStamp(stamp_type_t, time){
	format_type_ = WRF_FORMAT_THREE;
	data_type_ = data_type_t;
}

WrfDiscreteDataStamp::~WrfDiscreteDataStamp(){

}

bool WrfDiscreteDataStamp::LoadData(const char* file_name){
	std::ifstream input(file_name);

	if ( input.good() ){
		getline(input, header_info_);

		int temp_year, temp_month, temp_day, temp_hour, temp_level;
		int iso_num;
		float iso_value, smooth_factor, bold_factor;
		int edge_point_num;
		float longitude, latitude;
		int element_number, site_number;
		input >> temp_year >> temp_month >> temp_day >> temp_hour >> temp_level;
		input >> iso_num;
		for ( int i = 0; i < iso_num; ++i ) input >> iso_value;
		input >> smooth_factor >> bold_factor;
		input >> edge_point_num;
		for ( int i = 0; i < edge_point_num; ++i ) input >> longitude >> latitude;
		input >> element_number;
		input >> site_number;

		for ( int i = 0; i < site_number; ++i ){
			DiscreteDataRecord record;
			
			std::string str1, str2;
			input >> record.site_id >> record.longitude >> record.latitude >> str1 >> str2;
			record.value1 = (float)atof(str1.c_str());
			record.value2 = (float)atof(str2.c_str());

			if ( abs(record.value2 - 9999) < 1 ) continue;

			values.push_back(record);
		}

		input.close();
	} else {
		return false;
	}

	return true;
}

WrfGridDataStamp::WrfGridDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, const char* file_name, WrfGeneralDataStampType data_type_t)
	: WrfDataStamp(stamp_type_t, time){
	format_type_ = WRF_FORMAT_FOUR;
	data_type_ = data_type_t;

	if ( !LoadData(file_name) ){
		stamp_type_ = WRF_ERROR_STAMP;
	}
}

WrfGridDataStamp::WrfGridDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, WrfGeneralDataStampType data_type_t)
	: WrfDataStamp(stamp_type_t, time){
	format_type_ = WRF_FORMAT_FOUR;
	data_type_ = data_type_t;
}

WrfGridDataStamp::~WrfGridDataStamp(){

}

bool WrfGridDataStamp::LoadData(const char* file_name){
	std::ifstream input(file_name);

	if ( input.good() ){
		getline(input, header_info_);

		int temp_year, temp_month, temp_day, temp_hour, temp_level, temp_exposion_time;
		float iso_space, start_iso, end_iso, smooth_factor, bold_factor;

		input >> temp_year >> temp_month >> temp_day >> temp_hour >> temp_exposion_time >> temp_level;
		input >> longitude_grid_space >> latitude_grid_space;
		input >> start_longitude >> end_longitude;
		input >> start_latitude >> end_latitude;
		input >> longitude_grid_number >> latitude_grid_number;
		input >> iso_space >> start_iso >> end_iso >> smooth_factor >> bold_factor;

		values.resize(longitude_grid_number * latitude_grid_number);
		for ( int i = 0; i < longitude_grid_number * latitude_grid_number; ++i ) input >> values[i];

		input.close();
	} else {
		return false;
	}

	return true;
}