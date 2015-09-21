#include "wrf_common.h"
#include <fstream>

//------------------------------------- WrfTime ------------------------------------------------
WrfTime::WrfTime(){

}

WrfTime::WrfTime(int year_t, int month_t, int day_t, int hour_t){
	year = year_t;
	month = month_t;
	day = day_t;
	hour = hour_t;
}

WrfTime::WrfTime(const WrfTime& time){
	year = time.year;
	month = time.month;
	day = time.day;
	hour = time.hour;
}

WrfTime::~WrfTime(){
}

//------------------------------------- WrfValueMap ------------------------------------------------
WrfGridValueMap::WrfGridValueMap(){

}

WrfGridValueMap::~WrfGridValueMap(){

}

void WrfGridValueMap::Save(const char* file_name){
	std::ofstream output(file_name, std::ios::binary);

	if ( output.good() ){
		output.write((char*)&start_longitude, sizeof(float));
		output.write((char*)&end_longitude, sizeof(float));
		output.write((char*)&start_latitude, sizeof(float));
		output.write((char*)&end_latitude, sizeof(float));
		output.write((char*)&longitude_grid_number, sizeof(int));
		output.write((char*)&latitude_grid_number, sizeof(int));
		output.write((char*)&longitude_grid_space, sizeof(float));
		output.write((char*)&latitude_grid_space, sizeof(float));

		output.write((char*)&values[0], values.size() * sizeof(float));
		output.close();
	}
}

void WrfGridValueMap::Load(const char* file_name){
	std::ifstream input(file_name, std::ios::binary);

	if ( input.good() ){
		input.read((char*)&start_longitude, sizeof(float));
		input.read((char*)&end_longitude, sizeof(float));
		input.read((char*)&start_latitude, sizeof(float));
		input.read((char*)&end_latitude, sizeof(float));
		input.read((char*)&longitude_grid_number, sizeof(int));
		input.read((char*)&latitude_grid_number, sizeof(int));
		input.read((char*)&longitude_grid_space, sizeof(float));
		input.read((char*)&latitude_grid_space, sizeof(float));
		values.resize(longitude_grid_number * latitude_grid_number);
		input.read((char*)&values[0], longitude_grid_number * latitude_grid_number * sizeof(float));

		for (int i = 0; i < longitude_grid_number * latitude_grid_number; ++i ){
			values[i] *= 10;
			if ( values[i] > 1.0 ) values[i] = 1.0;
		}
	}
}