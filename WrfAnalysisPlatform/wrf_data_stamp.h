#ifndef WRF_DATA_STAMP_H_
#define WRF_DATA_STAMP_H_

#define PIE 3.14159

#include <vector>
#include <string>
#include "wrf_common.h"

enum WrfDataStampType
{
	WRF_ERROR_STAMP = 0x0,
	WRF_NUMERICAL_STAMP,
	WRF_HISTORICAL_STAMP,
};

enum WrfDataFormatType
{
	WRF_UNKNOWN_FORMAT = 0x0,
	WRF_FORMAT_ONE,
	WRF_FORMAT_TWO,
	WRF_FORMAT_THREE,
	WRF_FORMAT_FOUR
};

enum WrfGeneralDataStampType
{
	WRF_RAIN = 0x0,
	WRF_PRESSURE,
	WRF_TEMPERATURE_850HPA,
	WRF_HEIGHT_850HPA,
	WRF_RH_850HPA,
	WRF_WIND_X_850HPA,
	WRF_WIND_Y_850HPA,
	WRF_WIND_850HPA
};

struct SurfaceSiteRecord
{
	long site_id;
	float longitude, latitude, altitude;
	float wind_dir, wind_speed, pressure;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & site_id;
		ar & longitude;
		ar & latitude;
		ar & altitude;
		ar & wind_dir;
		ar & wind_speed;
		ar & pressure;
	}
};

struct HighAltitudeRecord
{
	long site_id;
	float longitude, latitude, altitude;
	float height, temper, dew_point, y_speed, x_speed;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & site_id;
		ar & longitude;
		ar & latitude;
		ar & altitude;
		ar & height;
		ar & temper;
		ar & dew_point;
		ar & y_speed;
		ar & x_speed;
	}
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

class WrfDataStamp
{
public:
	WrfDataStamp() {}
	WrfDataStamp(WrfDataStampType stamp_type_t, WrfTime& time){
		format_type_ = WRF_UNKNOWN_FORMAT;
		stamp_type_ = stamp_type_t;
		time_ = time;
		header_info_ = "";
	}
	~WrfDataStamp(){}

	WrfDataStampType stamp_type() const { return stamp_type_; }
	WrfDataFormatType format_type() const { return format_type_; }
	WrfTime time() const { return time_; }
	const std::string& header_info() const { return header_info_; }

	friend class boost::serialization::access;
	template< class Archive >
	void serialize(Archive& ar, const unsigned int version){
		ar & format_type_;
		ar & header_info_;
		ar & stamp_type_;
		ar & time_;
	}

protected:
	WrfDataFormatType format_type_;
	std::string header_info_;
	WrfDataStampType stamp_type_;
	WrfTime time_;

	virtual bool LoadData(const char* file_name){ return false; }
};

class WrfSurfaceDataStamp : public WrfDataStamp
{
public:
	WrfSurfaceDataStamp() {}
	WrfSurfaceDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, const char* file_name);
	~WrfSurfaceDataStamp();

	std::vector< SurfaceSiteRecord > values;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & boost::serialization::base_object< WrfDataStamp >(*this);
		ar & values;
	}

protected:
	virtual bool LoadData(const char* file_name);
};

class WrfHighAltitudeDataStamp : public WrfDataStamp
{
public:
	WrfHighAltitudeDataStamp() {}
	WrfHighAltitudeDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, const char* file_name);
	~WrfHighAltitudeDataStamp();

	std::vector< HighAltitudeRecord > values;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & boost::serialization::base_object< WrfDataStamp >(*this);
		ar & values;
	}

protected:
	virtual bool LoadData(const char* file_name);
};

class WrfDiscreteDataStamp : public WrfDataStamp
{
public:
	WrfDiscreteDataStamp();
	WrfDiscreteDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, const char* file_name, WrfGeneralDataStampType data_type_t);
	WrfDiscreteDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, WrfGeneralDataStampType data_type_t);
	~WrfDiscreteDataStamp();

	std::vector< DiscreteDataRecord > values;
	
	WrfGeneralDataStampType data_type() const { return data_type_; }

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & boost::serialization::base_object< WrfDataStamp >(*this);
		ar & values;
		ar & data_type_;
	}

protected:
	virtual bool LoadData(const char* file_name);

private:
	WrfGeneralDataStampType data_type_;
};

class WrfGridDataStamp : public WrfDataStamp
{
public:
	WrfGridDataStamp() {}
	WrfGridDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, const char* file_name, WrfGeneralDataStampType data_type_t);
	WrfGridDataStamp(WrfDataStampType stamp_type_t, WrfTime& time, WrfGeneralDataStampType data_type_t);
	~WrfGridDataStamp();

	float longitude_grid_space, latitude_grid_space;
	float start_longitude, end_longitude, start_latitude, end_latitude;
	int latitude_grid_number, longitude_grid_number;

	std::vector< float > values;

	WrfGeneralDataStampType data_type() const { return data_type_; }

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & boost::serialization::base_object< WrfDataStamp >(*this);
		ar & values;
		ar & data_type_;
		ar & longitude_grid_space;
		ar & latitude_grid_space;
		ar & start_longitude;
		ar & start_latitude;
		ar & end_longitude;
		ar & end_latitude;
		ar & latitude_grid_number;
		ar & longitude_grid_number;
	}

protected:
	virtual bool LoadData(const char* file_name);

private:
	WrfGeneralDataStampType data_type_;
};

enum WrfDataRecordSetType
{
	WRF_T639 = 0x0,
	WRF_EC_FINE,
	WRF_NCEP,
	WRF_JAPAN_GSM,
	WRF_SCENE
};

class WrfDataRecordSet
{
public:
	WrfDataRecordSet() {}
	WrfDataRecordSet(WrfDataRecordSetType type_t) { type = type_t; }
	~WrfDataRecordSet() {}

	WrfTime time;
	WrfDataRecordSetType type;
	std::vector< WrfDataRecord* > values;
	std::vector< WrfDataRecord* > his_values;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & time;
		ar & type;
		ar & values;
		ar & his_values;
	}
};

typedef std::map< int, WrfDataRecordSet* > RecordSetMap;
typedef std::map< int, std::vector< WrfDataStamp* >* > DataStampMap;
typedef std::map< int, std::vector< std::vector< float > >* > WeightMap;

#endif