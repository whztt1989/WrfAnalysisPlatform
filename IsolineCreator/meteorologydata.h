#ifndef METEOROLOGYDATA_H
#define METEOROLOGYDATA_H

#include "isolinecreatorconfig.h"
#include <string>
/**
 * 存储气象数据的类
 * @author wumin
 * @date 2012/09/09
 */
class EXPORT_CLASS MeteorologyData 
{
public:
	MeteorologyData();
	~MeteorologyData();

	/**
	 * 从文件中读取气象数据
	 * @return @c true if success
	 * @author wumin
	 * @date 2012/09/09
	 */
	bool load( std::string fileName );

public:
	int year;//年
	int month;//月
	int day;//日
	int hour;//时
	int lifecycle;//时效
	int level;//层次
	float longitudeGridSpace;//经度格距
	float latitudeGridSpace;//纬度格距
	float startLongitude;//起始经度
	float endLongitude;//终止经度
	float startLatitude;//起始纬度
	float endLatitude;//终止纬度
	int latitudeGridNumber;//纬向格点数，即一个纬线圈上经度格点的数量 361
	int longitudeGridNumber;//经向格点数，即经线方向上纬度格点的数量  281
	float isolineSpace;//等值线间隔，即等值线起始值和终止值的等差数列公差
	float isolineStartValue;//需要分析的等值线起始值
	float isolineEndValue;//需要分析的等值线终止值
	float smoothFactor;//平滑系数(暂不使用)
	float bold;//加粗线值(暂不使用)

	float* pValue; //网格点的数据

	float maxValue; //数据的最大值
	float minValue; //数据的最小值
};

#endif