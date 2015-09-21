#ifndef WRF_DATA_CONVERTER_H_
#define WRF_DATA_CONVERTER_H_

// This is used convert data from site based to grid based

#define INTERPOLATION_RADIUS 2.5

#include <vector>
#include "wrf_data_stamp.h"

class WrfDataConverter
{
public:
	WrfDataConverter();
	~WrfDataConverter();

	void set_longitude_grid_space(float space) { longitude_grid_space_ = space; }
	void set_latitude_grid_space(float space) { latitude_grid_space_ = space; }
	void set_start_longitude(float value) { start_longitude_ = value; }
	void set_end_longitude(float value) { end_longitude_ = value; }
	void set_start_latitude(float value) { start_latitude_ = value; }
	void set_end_latitude(float value) { end_latitude_ = value; }
	void set_longtitude_grid_number(int value) { longitude_grid_number_ = value; }
	void set_latitude_grid_number(int value) { latitude_grid_number_ = value; }

	WrfDataRecordSet* Convert(const std::vector< WrfDataStamp* >& numerical_stamps, const std::vector< WrfDataStamp* >& historical_stamps, 
		const std::vector< WrfDataStamp* >& base_stamps, WrfDataRecordSetType type);
	void Convert(WrfHighAltitudeDataStamp* high_stamp, std::vector< WrfDataStamp* >& data_stamps);
	WrfGridDataStamp* ConvertT(WrfDiscreteDataStamp* discrete_stamp);
	// Convert the discrete stamp to grid stamp according to the radius
	WrfGridDataStamp* Convert(WrfDiscreteDataStamp* discrete_stamp, float radius);
	// Interpolate the grid stamp to get the value at site
	WrfDiscreteDataStamp* Convert(WrfGridDataStamp* grid_stamp, WrfDiscreteDataStamp* template_stamp, float radius);
	// Convert between grid stamp to fit the grid size
	WrfGridDataStamp* Convert(WrfGridDataStamp* grid_stamp);
	// Convert a grid stamp to a new one based on the template size
	WrfGridDataStamp* Convert(WrfGridDataStamp* grid_stamp, WrfGridDataStamp* template_stamp);
	// The whole process of convert a site based stamp to grid based stamp
	WrfGridDataStamp* Interpolate(WrfDiscreteDataStamp* discrete_stamp, WrfGridDataStamp* base_stamp, float stop_value);

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & latitude_grid_space_;
		ar & longitude_grid_space_;
		ar & longitude_grid_number_;
		ar & latitude_grid_number_;
		ar & start_longitude_;
		ar & end_longitude_;
		ar & start_latitude_;
		ar & end_latitude_;
	}

private:
	float longitude_grid_space_, latitude_grid_space_;
	float start_longitude_, end_longitude_, start_latitude_, end_latitude_;
	int latitude_grid_number_, longitude_grid_number_;

	float InterpolateScalar(WrfGridDataStamp* grid_stamp, float longitude, float latitude, float radius);
	void InterpolateVector2D(WrfGridDataStamp* grid_stamp, float longitude, float latitude, float radius, float& x, float& y);

	float Dis(float x1, float y1, float x2, float y2);
};

#endif