#pragma once
#include "isotools.h"
#include<string>
#include<vector>
#include "isolinecreatorconfig.h"

using namespace std;

#define MAX_VALUE 10000000
#define MIN_VALUE -10000000

class EXPORT_CLASS IsolineCreator
{
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

	float maxValue;//最大值 重要！
	float minValue;//最小值 重要！
public:
	vector<vector<float> > value;
public:
	IsolineCreator(){};
	/************************************************************************/
	/* Funciton:   readGridData     
	 * Description: 读入网格数据
	 * Input:
	    startLongitude：起始经度
		longitudeGridSpace：经度格距
		longitudeGridNumber：经向格点数，即经线方向上纬度格点的数量  281
		startLatitude：起始纬度
		latitudeGridSpace：纬度格距
		latitudeGridNumber：纬向格点数，即一个纬线圈上经度格点的数量 361
		isolineStartValue：需要分析的等值线起始值
		isolineSpace：等值线间隔，即等值线起始值和终止值的等差数列公差
		data：网格数据
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date： 2012-8-30
	/************************************************************************/
	virtual bool readGridData(float startLongitude, float longitudeGridSpace, int longitudeGridNumber,float startLatitude,float latitudeGridSpace,int  latitudeGridNumber, float isolineStartValue,float isolineSpace, const float * data );
	/************************************************************************/
	/* Funciton:   generateContourLine     
	 * Description: 根据气候文件，将有效范围内的等值线值放入isoValues中，并将生成的等值线各坐标放入pathsPoints中
	 * Input:
		fileName: 气候文件名
		isoValues: 保存要绘制的各等值线的 isovalue值
		pathsPoints： 二维数组，保存生成的不同等值线上各点坐标
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date： 2012-8-24
	/************************************************************************/
	virtual void generateContourLine(vector<float> &isolineValueVector,vector< vector<isotools::Point2D> > &isolineList);

	/************************************************************************/
	/* Funciton:   generateContourLine     
	 * Description: 根据气候文件，将有效范围内的等值线值放入isoValues中，并将生成的等值线 放入pathsLinesV中
	 * Input:
		fileName: 气候文件名
		isoValues: 保存要绘制的各等值线的 isovalue值
		pathsLinesV： 二维数组，保存生成的不同等值的等值线集合 
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date： 2012-9-11
	/************************************************************************/
	virtual void generateContourLine(vector<float> &isoValues,vector< vector<isotools::Isoline> > &pathsLinesV);
	/************************************************************************/
	/* Funciton:   generateContourLineWithIsovalue       
	 * Description: 根据指定的等值线值生成的等值线，将其各点坐标放入pPoints中
	 * Input:
		isovalue: 指定的等值线值
		pPoints： 保存生成的等值线上各点坐标
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date： 2012-8-24
	/************************************************************************/
	virtual void generateContourLineWithIsovalue(float isovalue,vector<isotools::Point2D> &isoline );
	/************************************************************************/
	/* Funciton:   generateContourLineWithIsovalue       
	 * Description: 根据指定的等值线值生成的等值线，放入Isoline 数组 pathsLines 中
	 * Input:
		isovalue: 指定的等值线值
		pathsLines： 保存生成的等值线 
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date： 2012-9-11
	/************************************************************************/
	virtual void IsolineCreator::generateContourLineWithIsovalue(float isovalue,vector<isotools::Isoline> &pathsLines );
};