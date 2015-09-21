#include "wrf_data_loader.h"
#include <fstream>
#include <iostream>

WrfDataLoader* WrfDataLoader::instance_ = 0;

WrfDataLoader* WrfDataLoader::GetInstance(){
    if ( instance_ == 0 ){
        instance_ = new WrfDataLoader;
    }

    return instance_;
}

bool WrfDataLoader::DeleteInstance(){
    if ( instance_ != 0 ){
        delete instance_;
        return true;
    }
    return false;
}

WrfDataLoader::WrfDataLoader(){

}

WrfDataLoader::~WrfDataLoader(){

}

void WrfDataLoader::SetRootPath(std::string& root_path){
    data_root_path_ = root_path;
}

WrfValueMap* WrfDataLoader::LoadData(WrfTime& time, WrfModelType model_type, WrfElementType element_type, int exposion_time){
    std::string file_name;
    char temp_str[4];
    sprintf_s(temp_str, 4, "%03d", exposion_time);
    std::string exposion_end = "." + std::string(temp_str);

    switch (model_type){
    case WRF_OBSERVATION:
        file_name = "G:/VisData/meteorology/data/surface/";
        break;
    case WRF_EC_FINE:
        file_name = data_root_path_ + "newecmwf_grib/" + "newecmwf_grib_" + time.ToYMString() + "/";
        break;
    default:
        break;
    }

    switch (element_type){
    case WRF_RAIN_24_HOURS:
        if ( model_type == WRF_EC_FINE ){
            file_name += "rain24/rain02/" + time.ToString() + exposion_end;
            return LoadFormatFourData(file_name);
        } else {
            file_name += "r24-8/" + time.ToString() + ".000";
            return LoadFormatThreeData(file_name);
        }
        break;
    default:
        break;
    }

    return NULL;
}

void WrfDataLoader::LoadData(WrfTime& begin_time, WrfTime& end_time, WrfModelType model_type, WrfElementType element_type, std::vector< WrfValueMap* >& value_maps, int exposion_time){
    WrfTime temp_time = begin_time;

    value_maps.clear();
    while ( temp_time <= end_time ){
        WrfValueMap* temp_map = LoadData(temp_time, model_type, element_type, exposion_time);
        if ( temp_map != NULL ){
            temp_map->map_time = temp_time;
            temp_map->model_type = model_type;
            temp_map->element_type = element_type;
            temp_map->exposion_time = exposion_time;

            value_maps.push_back(temp_map);
        } else {
            std::cout << "Lost of data at time: " << temp_time.ToString() << "   Model Type: " << model_type << "   Element Type: " << element_type << std::endl;
        }

        temp_time.AddHour(exposion_time);
    }
}

WrfGridValueMap* WrfDataLoader::LoadFormatFourData(std::string& file_name){
    std::ifstream input(file_name);
    float min_value = 1e10;
    float max_value = -1e10;

    if ( input.good() ){
        WrfGridValueMap* value_stamp = new WrfGridValueMap;

        getline(input, value_stamp->header_info);

        int temp_year, temp_month, temp_day, temp_hour, temp_level, temp_exposion_time;
        float iso_space, start_iso, end_iso, smooth_factor, bold_factor;

        input >> temp_year >> temp_month >> temp_day >> temp_hour >> temp_exposion_time >> temp_level;
        input >> value_stamp->longitude_grid_space >> value_stamp->latitude_grid_space;
        input >> value_stamp->start_longitude >> value_stamp->end_longitude;
        input >> value_stamp->start_latitude >> value_stamp->end_latitude;
        input >> value_stamp->longitude_grid_number >> value_stamp->latitude_grid_number;
        input >> iso_space >> start_iso >> end_iso >> smooth_factor >> bold_factor;

        value_stamp->values.resize(value_stamp->longitude_grid_number * value_stamp->latitude_grid_number);
        for ( int i = 0; i < value_stamp->longitude_grid_number * value_stamp->latitude_grid_number; ++i ) {
            input >> value_stamp->values[i];
            if ( value_stamp->values[i] > max_value ) max_value = value_stamp->values[i];
            if ( value_stamp->values[i] < min_value ) min_value = value_stamp->values[i];
        }

        input.close();

        std::cout << value_stamp->header_info << " " << max_value << " " << min_value << std::endl;

        return value_stamp;
    } else {
        return NULL;
    }
}

WrfDiscreteValueMap* WrfDataLoader::LoadFormatThreeData(std::string& file_name){
    std::ifstream input(file_name);

    float min_value = 1e10;
    float max_value = -1e10;

    if ( input.good() ){
        WrfDiscreteValueMap* value_map = new WrfDiscreteValueMap;

        getline(input, value_map->header_info);

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

            if ( record.longitude > value_map->max_longitude ) value_map->max_longitude = record.longitude;
            if ( record.longitude < value_map->min_longitude ) value_map->min_longitude = record.longitude;
            if ( record.latitude > value_map->max_latitude ) value_map->max_latitude = record.latitude;
            if ( record.latitude < value_map->min_latitude ) value_map->min_latitude = record.latitude;
            if ( record.value2 > max_value ) max_value = record.value2;
            if ( record.value2 < min_value ) min_value = record.value2;

            value_map->values.push_back(record);
        }

        input.close();

        std::cout << value_map->header_info << " " << max_value << " " << min_value << std::endl;

        return value_map;
    } else {
        return NULL;
    }
}