#include "wrf_data_loader.h"
#include <fstream>
#include <iostream>
#include <QtXml/QDomDocument>
#include <QFile>
#include <QDebug>


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
    InitLoaderInfo();

    LoadFormatNcData(std::string("E:/Weather Forcast/Data/apcpusdaily_refcstens_1979010100-2004123100.nc"));
}

WrfDataLoader::~WrfDataLoader(){

}

void WrfDataLoader::InitLoaderInfo(){
    path_infos_.resize(7);
    format_infos_.resize(7);
    for ( int i = 0; i < path_infos_.size(); ++i ) {
        path_infos_[i].resize(9, "");
        format_infos_[i].resize(9, -1);
    }

    QFile file("./Resources/data_info.xml");

    QDomDocument dom_document;

    if ( file.open(QFile::ReadOnly | QFile::Text ) ){
        QString error_str;
        int error_line;
        int error_column;

        if (!dom_document.setContent(&file, false, &error_str, &error_line, &error_column)){
            qDebug() << "Error: Parse error at line " << error_line << ","
                << "colume " << error_column << ": "
                << qPrintable(error_str);
            return;
        }

        QDomElement root = dom_document.documentElement();
        if(root.tagName() != "PathInfo"){
            qDebug() << "Error: Not a path information file";
            return;
        }

        // Space length
        QDomElement root_path_element = root.firstChildElement("RootPath");
        QString root_path = root_path_element.attribute("value");
        data_root_path_ = std::string(root_path.toLocal8Bit().data());

        QDomElement model_path_element = root.firstChildElement("Model");
        while ( !model_path_element.isNull() ){
            QString model_path = model_path_element.attribute("value");
            QString model_name = model_path_element.attribute("name");

            WrfModelType model_type;
            if ( model_name.compare("ec_fine") == 0 ){
                model_type = WRF_EC_FINE;
            } else if ( model_name.compare("japan_gsm") == 0 ){
                model_type = WRF_JAPAN_GSM;
            } else if ( model_name.compare("t639") == 0 ){
                model_type = WRF_T639;
            } else {
                model_type = WRF_UNKNOWN_MODEL;
            }

            std::string temp_path = std::string(model_path.toLocal8Bit().data());
            LoadModelPath(model_path_element, temp_path, path_infos_[model_type], format_infos_[model_type]);

            model_path_element = model_path_element.nextSiblingElement();
        }
    }
}

void WrfDataLoader::LoadModelPath(QDomElement& dom_element, std::string& model_root, std::vector< std::string >& info, std::vector< int >& format_info){
    QDomNode element_node = dom_element.firstChild();

    while ( !element_node.isNull() ){
        QString element_name = element_node.toElement().attribute("name");
        QString element_path = element_node.toElement().attribute("value");
        QString format_str = element_node.toElement().attribute("format");

        WrfElementType element_type;
        if ( element_name.compare("rain24") == 0 )
            element_type = WRF_RAIN_24_HOURS;
        else if ( element_name.compare("temperature_500hpa") == 0 )
            element_type = WRF_TEMPERATURE_500HPA;
        else if ( element_name.compare("temperature_850hpa") == 0 )
            element_type = WRF_TEMPERATURE_850HPA;
        else if ( element_name.compare("height_500hpa") == 0 )
            element_type = WRF_HEIGHT_500HPA;
        else if ( element_name.compare("height_850hpa") == 0 )
            element_type = WRF_HEIGHT_850HPA;
        else if ( element_name.compare("relative_humidity_500hpa") == 0 )
            element_type = WRF_RELATIVE_HUMIDITY_500HPA;
        else if ( element_name.compare("relative_humidity_850hpa") == 0 )
            element_type = WRF_RELATIVE_HUMIDITY_850HPA;
        else
            element_type = WRF_UNKNOWN_ELEMENT;

        info[element_type] = model_root + std::string(element_path.toLocal8Bit().data());
        format_info[element_type] = format_str.toInt();

        element_node = element_node.nextSibling();
    }
}

WrfValueMap* WrfDataLoader::LoadData(WrfTime& time, WrfModelType model_type, WrfElementType element_type, int exposion_time){
    std::string file_name;
    char temp_str[4];
    sprintf_s(temp_str, 4, "%03d", exposion_time);
    std::string exposion_end = "." + std::string(temp_str);
    file_name = data_root_path_ + path_infos_[model_type][element_type] + time.ToString() + exposion_end;

    WrfValueMap* result_map = NULL;
    switch ( format_infos_[model_type][element_type] ){
    case 3:
        result_map = LoadFormatThreeData(file_name);
        break;
    case 4:
        result_map = LoadFormatFourData(file_name);
        break;
    default:
        break;
    }
    if ( result_map != NULL ){
        result_map->model_type = model_type;
        result_map->element_type = element_type;
    }

    return result_map;
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

        getline(input, value_stamp->title);

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

        std::cout << value_stamp->title << " " << max_value << " " << min_value << std::endl;

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

        getline(input, value_map->title);

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

        std::cout << value_map->title << " " << max_value << " " << min_value << std::endl;

        return value_map;
    } else {
        return NULL;
    }
}

void WrfDataLoader::LoadFormatNcData(std::string& file_name){
}