#ifndef WRF_COMMON_H_
#define WRF_COMMON_H_

#include <vector>
#include <QtGui/QColor>
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/hash_map.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/base_object.hpp"
#include "boost/serialization/export.hpp"

class WrfTime
{
public:
	WrfTime();
	WrfTime(int year_t, int month_t, int day_t, int hour_t);
	WrfTime(const WrfTime& time);
	~WrfTime();

	const WrfTime& operator= (const WrfTime& time){
		year = time.year;
		month = time.month;
		day = time.day;
		hour = time.hour;

		return *this;
	}

	int ToInt(){
		return day; 
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

class WrfGridValueMap
{
public:
	WrfGridValueMap();
	~WrfGridValueMap();

	float longitude_grid_space, latitude_grid_space;
	float start_longitude, end_longitude, start_latitude, end_latitude;
	int latitude_grid_number, longitude_grid_number;

	float max_value, min_value;
	std::vector< float > values;
	std::vector< float > weight;
	std::vector< float > info;
	int level;

	void Save(const char* file_name);
	void Load(const char* file_name);

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
		ar & weight;
		ar & info;
		ar & level;
	}
};

struct SimilarityRecord
{
	float max_value;
	float min_value;
	float mean_value;
};

class WrfGridSimilarityMap
{
public:
	WrfGridSimilarityMap() {}
	~WrfGridSimilarityMap() {}

	float longitude_grid_space, latitude_grid_space;
	float start_longitude, end_longitude, start_latitude, end_latitude;
	int latitude_grid_number, longitude_grid_number;

	float min_value, max_value;
	std::vector< SimilarityRecord > values;

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


class WrfDataRecord
{
public:
	WrfDataRecord(){
		rain = 0;
		relative_humidity_850hpa = 0;
		pressure = 0;
		y_speed_850hpa = 0;
		x_speed_850hpa = 0;
		height_850hpa = 0;
		temperature_850hpa = 0;
	}

	WrfDataRecord(float r, float rh, float sap, float ws, float wd, float h850, float t850){
		rain = r;
		relative_humidity_850hpa = rh;
		pressure = sap;
		y_speed_850hpa = ws;
		x_speed_850hpa = wd;
		height_850hpa = h850;
		temperature_850hpa = t850;
	}

	WrfDataRecord(const WrfDataRecord& record){
		rain = record.rain;
		relative_humidity_850hpa = record.relative_humidity_850hpa;
		pressure = record.pressure;
		y_speed_850hpa = record.y_speed_850hpa;
		x_speed_850hpa = record.x_speed_850hpa;
		height_850hpa = record.height_850hpa;
		temperature_850hpa = record.temperature_850hpa;
	}

	~WrfDataRecord() {}

	const WrfDataRecord& operator=(const WrfDataRecord& record){
		rain = record.rain;
		relative_humidity_850hpa = record.relative_humidity_850hpa;
		pressure = record.pressure;
		y_speed_850hpa = record.y_speed_850hpa;
		x_speed_850hpa = record.x_speed_850hpa;
		height_850hpa = record.height_850hpa;
		temperature_850hpa = record.temperature_850hpa;

		return *this;
	}

	friend inline const WrfDataRecord operator-(const WrfDataRecord& r1, const WrfDataRecord& r2){
		return WrfDataRecord(r1.rain - r2.rain, r1.relative_humidity_850hpa - r2.relative_humidity_850hpa, r1.pressure - r2.pressure, r1.y_speed_850hpa - r2.y_speed_850hpa, r1.x_speed_850hpa - r2.x_speed_850hpa, r1.height_850hpa - r2.height_850hpa, r1.temperature_850hpa - r2.temperature_850hpa);
	}

	friend inline const WrfDataRecord operator+(const WrfDataRecord& r1, const WrfDataRecord& r2){
		return WrfDataRecord(r1.rain + r2.rain, r1.relative_humidity_850hpa + r2.relative_humidity_850hpa, r1.pressure + r2.pressure, r1.y_speed_850hpa + r2.y_speed_850hpa, r1.x_speed_850hpa + r2.x_speed_850hpa, r1.height_850hpa + r2.height_850hpa, r1.temperature_850hpa + r2.temperature_850hpa);
	}

	void operator*=(float scale){
		rain *= scale;
		relative_humidity_850hpa *= scale;
		pressure *= scale;
		y_speed_850hpa *= scale;
		x_speed_850hpa *= scale;
		height_850hpa *= scale;
		temperature_850hpa *= scale;
	}
	
	void ToAbs(){
		rain = abs(rain);
		relative_humidity_850hpa = abs(relative_humidity_850hpa);
		pressure = abs(pressure);
		y_speed_850hpa = abs(y_speed_850hpa);
		x_speed_850hpa = abs(x_speed_850hpa);
		height_850hpa = abs(height_850hpa);
		temperature_850hpa = abs(temperature_850hpa);
	}

	float ToSum(){
		return rain + relative_humidity_850hpa + pressure + x_speed_850hpa + y_speed_850hpa + height_850hpa + temperature_850hpa;
	}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & rain;
		ar & relative_humidity_850hpa;
		ar & pressure;
		ar & y_speed_850hpa;
		ar & x_speed_850hpa;
		ar & height_850hpa;
		ar & temperature_850hpa;
	}

	float rain;
	float relative_humidity_850hpa;
	float pressure;
	float y_speed_850hpa;
	float x_speed_850hpa;
	float height_850hpa;
	float temperature_850hpa;
};

struct WrfParallelRecord
{
	float data[10];
	float alpha;
	int grid_index;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & data;
		ar & alpha;
	}
};

struct WrfParallelModel
{
	float r, g, b;
	std::string model_name;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & r;
		ar & g;
		ar & b;
		ar & model_name;
	}
};

class WrfParallelDataSet
{
public:
	WrfParallelDataSet() {}
	~WrfParallelDataSet() {}

	std::vector< WrfParallelModel > model_vec;
	std::vector< std::string > attrib_name_vec;
	std::vector< float > min_value_vec;
	std::vector< float > max_value_vec;
	std::vector< int > mapped_axis;
	std::vector< std::vector< WrfParallelRecord > > values;


	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & model_vec;
		ar & attrib_name_vec;
		ar & min_value_vec;
		ar & max_value_vec;
		ar & mapped_axis;
		ar & values;
	}
};

class LineChartRecord
{
public:
	LineChartRecord() {}
	~LineChartRecord() {}

	float alpha;
	std::vector< float > values;
	int grid_index;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & alpha;
		ar & values;
	}
};

class WrfLineChartDataSet
{
public:
	WrfLineChartDataSet() {}
	~WrfLineChartDataSet() {}

	int time_length;
	float max_value, min_value;
	std::string x_coor_name, y_coor_name, y1_coor_name;
	std::vector< float > colors;
	std::vector< std::string > line_names;
	std::vector< std::vector< LineChartRecord* > > values;
	float center_absolute_value;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & time_length;
		ar & max_value;
		ar & min_value;
		ar & x_coor_name;
		ar & y_coor_name;
		ar & y1_coor_name;
		ar & colors;
		ar & line_names;
		ar & values;
		ar & center_absolute_value;
	}
};

struct WrfMatrixChartRecord
{
	int x_index;
	int y_index;
	float value;
};

class WrfMatrixChartDataSet
{
public:
	WrfMatrixChartDataSet() {}
	~WrfMatrixChartDataSet() {}

	std::string x_coor_name, y_coor_name;
	int x_size, y_size;
	std::vector< WrfMatrixChartRecord > values; 
};



class WrfHistogramDataSet
{
public:
	WrfHistogramDataSet() {}
	~WrfHistogramDataSet() {}

	std::vector< int > grid_index;
	std::vector< float > model_rgb_color;
	std::vector< std::vector< std::vector< float > > > grid_model_weight;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & grid_index;
		ar & model_rgb_color;
		ar & grid_model_weight;
	}
};

#endif