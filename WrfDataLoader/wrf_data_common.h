#ifndef WRF_DATA_COMMON_H_
#define WRF_DATA_COMMON_H_

#include "wrf_common_global.h"
#include <string>
#include <vector>
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/hash_map.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/base_object.hpp"
#include "boost/serialization/export.hpp"

//template class WRF_DATA_EXPORT std::allocator< float >;
//template class WRF_DATA_EXPORT std::vector< float >;
//template class WRF_DATA_EXPORT std::allocator< char >;
//template class WRF_DATA_EXPORT std::basic_string< char >;

enum WrfModelType{
    WRF_UNKNOWN_MODEL = 0x0,
    WRF_OBSERVATION,
    WRF_T639,
    WRF_EC_FINE,
    WRF_JAPAN_GSM,
    WRF_NCEP,
    WRF_ELEMENT_UNCERTAINTY
};

enum WrfElementType{
    WRF_UNKNOWN_ELEMENT = 0x0,
    WRF_RAIN_12_HOURS,
    WRF_RAIN_24_HOURS,
    WRF_TEMPRATURE_500HPA,
    WRF_TEMPERATURE_850HPA,
    WRF_HEIGHT_500HPA,
    WRF_HEIGHT_850HPA,
    WRF_RELATIVE_HUMIDITY_500HPA,
    WRF_RELATIVE_HUMIDITY_850HPA,
    WRF_UNCERTAINTY
};

class WRFCOMMON_EXPORT WrfTime
{
public:
    WrfTime(){
        year = 0;
        month = 0;
        day = 0;
        hour = 0;
    }
    WrfTime(int year_t, int month_t, int day_t, int hour_t){
        year = year_t;
        month = month_t;
        day = day_t;
        hour = hour_t;
    }
    WrfTime(const WrfTime& time){
        year = time.year;
        month = time.month;
        day = time.day;
        hour = time.hour;
    }
    ~WrfTime(){}

    const WrfTime& operator = (const WrfTime& time){
        year = time.year;
        month = time.month;
        day = time.day;
        hour = time.hour;

        return *this;
    }

    void AddHour(int hour_t){
        hour += hour_t;
        if ( hour > 24 ){
            day += hour / 24;
            hour %= 24;
        }
        if ( month == 2 && day > 28 ){
            if ( year % 4 == 0 && year % 100 != 0 || year % 400 == 0 ){
                if ( day > 29 ){
                    month += 1;
                    day -= 29;
                }
            } else {
                month += 1;
                day -= 28;
            }
        } else if ( day > 30 ){
            if ( month == 4 || month == 6 || month == 9 ){
                month += 1;
                day -= 30;
            } else if ( day > 31 ){
                month += 1;
                day -= 31;
            }
        }

        if ( month > 12 ){
            year += 1;
            month -= 12;
        }
    }

    int ToInt() const{
        return hour + day * 100 + month * 10000 + year % 100 * 1000000; 
    }

    std::string ToString() const{
        char temp_str[9];
        sprintf_s(temp_str, 9, "%08d", ToInt());
        return std::string(temp_str);
    }

    std::string ToYMString() const{
        char temp_str[7];
        sprintf_s(temp_str, 7, "%06d", year * 100 + month);
        return std::string(temp_str);
    }

    friend inline bool operator < (const WrfTime& time1, const WrfTime& time2){
        return (time1.ToInt() < time2.ToInt());
    }

    friend inline bool operator > (const WrfTime& time1, const WrfTime& time2){
        return (time1.ToInt() > time2.ToInt());
    }

    friend inline bool operator <= (const WrfTime& time1, const WrfTime& time2){
        return (time1.ToInt() <= time2.ToInt());
    }
    
    friend inline bool operator >= (const WrfTime& time1, const WrfTime& time2){
        return (time1.ToInt() >= time2.ToInt());
    }

    friend inline bool operator == (const WrfTime& time1, const WrfTime& time2){
        return (time1.ToInt() == time2.ToInt());
    }

    friend class boost::serialization::access;
    template< class Archive >
    void serialize(Archive& ar, const unsigned int version){
        ar & year;
        ar & month;
        ar & day;
        ar & hour;
    }

    int year, month, day, hour;
};

class WRFCOMMON_EXPORT WrfValueMap
{
public:
    WrfValueMap() {
        model_type = WRF_UNKNOWN_MODEL;
        element_type = WRF_UNKNOWN_ELEMENT;

        map_time = WrfTime(0, 0, 0, 0);
        exposion_time = -1;
    }
    ~WrfValueMap() {}

    enum ValueMapType{
        UNKNOWN_VALUE_MAP = 0x0,
        DISCRETE_VALUE_MAP,
        GRID_VALUE_MAP
    };

    virtual ValueMapType type(){
        return UNKNOWN_VALUE_MAP;
    }

    friend class boost::serialization::access;
    template< class Archive >
    void serialize(Archive& ar, const unsigned int version){
        ar & model_type;
        ar & element_type;
        ar & header_info;

        ar & map_time;
        ar & exposion_time;
    }

    WrfModelType model_type;
    WrfElementType element_type;
    std::string header_info;

    WrfTime map_time;
    int exposion_time;
};

struct DiscreteDataRecord
{
    long site_id;
    float longitude, latitude, altitude;
    float value1, value2;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & site_id;
        ar & longitude;
        ar & latitude;
        ar & altitude;
        ar & value1;
        ar & value2;
    }
};

class WrfDiscreteValueMap : public WrfValueMap
{
public:
    WrfDiscreteValueMap() {
        min_longitude = 1e10;
        max_longitude = -1e10;
        min_latitude = 1e10;
        max_latitude = -1e10;
    }
    ~WrfDiscreteValueMap() {}

    std::vector< DiscreteDataRecord > values;
    float min_longitude, max_longitude, min_latitude, max_latitude;

    virtual WrfValueMap::ValueMapType type() {
        return WrfValueMap::DISCRETE_VALUE_MAP;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & boost::serialization::base_object< WrfValueMap >(*this);
        ar & values;
    }
};

class WRF_DATA_EXPORT WrfGridValueMap : public WrfValueMap
{
public:
    WrfGridValueMap(){
        longitude_grid_space = 0.0f;
        latitude_grid_space = 0.0f;
        start_longitude =  -1.0f;
        end_longitude = -1.0f;
        start_latitude = -1.0f;
        end_latitude = -1.0f;
        latitude_grid_number = 0;
        longitude_grid_space = 0;
        max_value = -1.0f;
        min_value = -1.0f;
        values.clear();
    }

    WrfGridValueMap(const WrfGridValueMap& value_map){
        this->model_type = value_map.model_type;
        this->element_type = value_map.element_type;
        this->header_info = value_map.header_info;

        this->longitude_grid_number = value_map.longitude_grid_number;
        this->latitude_grid_number = value_map.latitude_grid_number;
        this->longitude_grid_space = value_map.longitude_grid_space;
        this->latitude_grid_space = value_map.latitude_grid_space;
        this->start_longitude = value_map.start_longitude;
        this->start_latitude = value_map.start_latitude;
        this->end_longitude = value_map.end_longitude;
        this->end_latitude = value_map.end_latitude;

        this->max_value = value_map.max_value;
        this->min_value = value_map.min_value;
        this->values.assign(value_map.values.begin(), value_map.values.end());

        this->map_time = value_map.map_time;
        this->exposion_time = value_map.exposion_time;
    }

    ~WrfGridValueMap(){

    }

    static WrfGridValueMap* ConvertDiscrete2Grid(WrfDiscreteValueMap* discrete_map, WrfGridValueMap* template_map);
    static float Interpolate(WrfGridValueMap* value_map, float longitude, float latitude, float radius);
    static float Distance(float x1, float y1, float x2, float y2);

    WrfGridValueMap* ConvertDiscrete2Grid(WrfDiscreteValueMap* discrete_map);

    float longitude_grid_space, latitude_grid_space;
    float start_longitude, end_longitude, start_latitude, end_latitude;
    int latitude_grid_number, longitude_grid_number;

    float max_value, min_value;
    std::vector< float > values;

    const WrfGridValueMap& operator = (WrfGridValueMap& value_map){
        this->model_type = value_map.model_type;
        this->element_type = value_map.element_type;
        this->header_info = value_map.header_info;

        this->longitude_grid_number = value_map.longitude_grid_number;
        this->latitude_grid_number = value_map.latitude_grid_number;
        this->longitude_grid_space = value_map.longitude_grid_space;
        this->latitude_grid_space = value_map.latitude_grid_space;
        this->start_longitude = value_map.start_longitude;
        this->start_latitude = value_map.start_latitude;
        this->end_longitude = value_map.end_longitude;
        this->end_latitude = value_map.end_latitude;

        this->max_value = value_map.max_value;
        this->min_value = value_map.min_value;
        this->values.assign(value_map.values.begin(), value_map.values.end());

        this->map_time = value_map.map_time;
        this->exposion_time = value_map.exposion_time;

        return *this;
    }

    virtual WrfValueMap::ValueMapType type() {
        return WrfValueMap::GRID_VALUE_MAP;
    }

    friend class boost::serialization::access;
    template< class Archive >
    void serialize(Archive& ar, const unsigned int version){
        ar & longitude_grid_space;
        ar & latitude_grid_space;
        ar & start_longitude;
        ar & start_latitude;
        ar & end_longitude;
        ar & end_latitude;
        ar & latitude_grid_number;
        ar & longitude_grid_number;
        ar & max_value;
        ar & min_value;
        ar & values;
    }
};

#endif