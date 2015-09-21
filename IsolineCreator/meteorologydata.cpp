#include "meteorologydata.h"
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>


MeteorologyData::MeteorologyData()
{
	year = 0;
	month = 0;
	day = 0;
	hour = 0;
	lifecycle = 0;
	level = 0;
	longitudeGridSpace = 0.0f;
	latitudeGridSpace = 0.0f;
	startLongitude = 0.0f;
	endLongitude = 0.0f;
	startLatitude = 0.0f;
	endLatitude = 0.0f;
	latitudeGridNumber = 0;
	longitudeGridNumber = 0;
	isolineSpace = 0.0f;
	isolineStartValue = 0.0f;
	isolineEndValue = 0.0f;
	smoothFactor = 0.0f;
	bold = 0.0f;

	pValue = 0;

	maxValue = -std::numeric_limits<float>::max();
	minValue = std::numeric_limits<float>::max();
}

MeteorologyData::~MeteorologyData()
{
	if ( pValue ) delete [] pValue;
}

bool MeteorologyData::load( std::string fileName )
{
	std::ifstream infile( fileName.c_str() );
	if ( !infile )
	{
		std::cerr << "Fail to read data file" << std::endl;
		return false;
	}

	std::string line;
	// 根据第一行关键词“diamond”，判断数据文件合法性
	std::getline( infile, line );
	int pos = line.find("diamond");
	if ( pos == std::string::npos )
	{
		std::cerr << "Data file illegal" << std::endl;
		return false;
	}
	//读取第二行中的数据头信息
	std::getline( infile, line );
	std::istringstream linestream( line );
	linestream >> year;
	linestream >> month;
	linestream >> day;
	linestream >> hour;
	linestream >> lifecycle;
	linestream >> level;
	linestream >> longitudeGridSpace;
	linestream >> latitudeGridSpace;
	linestream >> startLongitude;
	linestream >> endLongitude;
	linestream >> startLatitude;
	linestream >> endLatitude;
	linestream >> latitudeGridNumber;
	linestream >> longitudeGridNumber;
	linestream >> isolineSpace;
	linestream >> isolineStartValue;
	linestream >> isolineEndValue;
	linestream >> smoothFactor;
	linestream >> bold;
	
	//从第三行开始读取网格点数据值
	pValue = new float[longitudeGridNumber*latitudeGridNumber];
	int i = 0;
	float temp;
	while ( infile >> temp )
	{
		pValue[i++] = temp;
		if ( temp > maxValue  ) maxValue = temp;
		if ( temp < minValue ) minValue = temp;
	}
	return true;
}