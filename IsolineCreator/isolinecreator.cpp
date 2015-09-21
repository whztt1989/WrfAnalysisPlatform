#include "isolinecreator.h"
#include "loaddata.h"
#include<ctime>
#include<iomanip>
#include "marchingsquares.h"


bool IsolineCreator::readGridData(float startLongitude, float longitudeGridSpace, int longitudeGridNumber,float startLatitude,float latitudeGridSpace,int  latitudeGridNumber, float isolineStartValue,float isolineSpace, const float * data )
{
	this->startLongitude = startLongitude; //起始经度
	this->longitudeGridSpace = longitudeGridSpace; //经度格距
	this->longitudeGridNumber = longitudeGridNumber; //经向格点数，即经线方向上纬度格点的数量  281
	this->endLongitude = this->startLongitude + (latitudeGridNumber-1)*longitudeGridSpace;

	this->startLatitude = startLatitude;//起始纬度
	this->latitudeGridSpace = latitudeGridSpace;//纬度格距
	this->latitudeGridNumber = latitudeGridNumber;//纬向格点数，即一个纬线圈上经度格点的数量 361
	this->endLatitude = this->startLatitude + (longitudeGridNumber-1)*latitudeGridSpace;

	this->isolineStartValue = isolineStartValue;
	this->isolineSpace = isolineSpace;
	
	maxValue = MIN_VALUE;
	minValue = MAX_VALUE;

	this->value.resize(longitudeGridNumber);
	//初始化value
	for(int i =0;i <longitudeGridNumber ;++i)//逐行
	{
		this->value[i].resize(latitudeGridNumber);
	}
	float tmpValue =0;
	for(int i =0;i < longitudeGridNumber;++i)//逐行
		for (int j=0;j<latitudeGridNumber;++j)//逐列
		{
			tmpValue = *(data++);
			this->value[i][j]=tmpValue;
			if(maxValue < tmpValue) maxValue = tmpValue;
			if(minValue > tmpValue) minValue = tmpValue;
		}
	return true;

}

void IsolineCreator::generateContourLine(vector<float> &isoValues,vector< vector<isotools::Point2D> > &pathsPoints)
{

	//计算时间
	//clock_t start,finish;//计算时间，使用时可以注释掉
	//double duration;//计算时间，使用时可以注释掉
	//cout<<"time begin to count:"<<endl;//计算时间，使用时可以注释掉
	//start = clock();//计算时间，使用时可以注释掉

	/*float isoValue = this->isolineStartValue;
	while (isoValue < this->minValue)
	{
		isoValue += this->isolineSpace;
	}
	
	while (isoValue < this->maxValue)
	{
		isoValues.push_back(isoValue);
		isoValue += this->isolineSpace;
	}*/

	//for (int i=0;i<3;++i)
    for (int i=0;i<isoValues.size();++i)
	{
		vector<isotools::Point2D> pPoints;
		marchingsquares::doMarchingSquares(value,isoValues[i],pPoints,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
		pathsPoints.push_back(pPoints);
	}
	//finish = clock();//计算时间，使用时可以注释掉
	//duration = (double)(finish-start)/CLOCKS_PER_SEC;//计算时间，使用时可以注释掉
	//cout<<"the total cost time is: "<<setiosflags(ios::fixed)<<setprecision(6)<< duration<<" seconds"<<endl;//计算时间，使用时可以注释掉
}

void IsolineCreator::generateContourLine(vector<float> &isoValues,vector< vector<isotools::Isoline> > &pathsLinesV)
{

	//计算时间
	//clock_t start,finish;//计算时间，使用时可以注释掉
	//double duration;//计算时间，使用时可以注释掉
	//cout<<"time begin to count:"<<endl;//计算时间，使用时可以注释掉
	//start = clock();//计算时间，使用时可以注释掉

	/*float isoValue = this->isolineStartValue;
	while (isoValue < this->minValue)
	{
		isoValue += this->isolineSpace;
	}

	while (isoValue < this->maxValue)
	{
		isoValues.push_back(isoValue);
		isoValue += this->isolineSpace;
	}*/
	//方法一：
	//for (int i=0;i<isoValues.size();++i)
	//{
	//	vector<isotools::Isoline> pLines;
	//	marchingsquares::doMarchingSquares(value,isoValues[i],pLines,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	//	pathsLines.push_back(pLines);
	//}
	//方法二：
	 marchingsquares::doMarchingSquares(value,isoValues,pathsLinesV,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	//finish = clock();//计算时间，使用时可以注释掉
	//duration = (double)(finish-start)/CLOCKS_PER_SEC;//计算时间，使用时可以注释掉
	//cout<<"the total cost time is: "<<setiosflags(ios::fixed)<<setprecision(6)<< duration<<" seconds"<<endl;//计算时间，使用时可以注释掉
}

void IsolineCreator::generateContourLineWithIsovalue(float isovalue,vector<isotools::Point2D> &pPoints )
{
	//clock_t start,finish; //计算时间，使用时可以注释掉
	//double duration;//计算时间，使用时可以注释掉
	//cout<<"time begin to count:"<<endl;//计算时间，使用时可以注释掉
	//start = clock();//计算时间，使用时可以注释掉
	//vector<MarchingSquares::Point2D> pPoints;
	marchingsquares::doMarchingSquares(value,isovalue,pPoints,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	//finish = clock();//计算时间，使用时可以注释掉
	//duration = (double)(finish-start)/CLOCKS_PER_SEC;//计算时间，使用时可以注释掉
	//cout<<"the total cost time is: "<<setiosflags(ios::fixed)<<setprecision(6)<< duration<<" seconds"<<endl;//计算时间，使用时可以注释掉
}

void IsolineCreator::generateContourLineWithIsovalue(float isovalue,vector<isotools::Isoline> &pathsLines )
{
	//clock_t start,finish; //计算时间，使用时可以注释掉
	//double duration;//计算时间，使用时可以注释掉
	//cout<<"time begin to count:"<<endl;//计算时间，使用时可以注释掉
	//start = clock();//计算时间，使用时可以注释掉
	//vector<MarchingSquares::Point2D> pPoints;
	marchingsquares::doMarchingSquares(value,isovalue,pathsLines,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	//finish = clock();//计算时间，使用时可以注释掉
	//duration = (double)(finish-start)/CLOCKS_PER_SEC;//计算时间，使用时可以注释掉
	//cout<<"the total cost time is: "<<setiosflags(ios::fixed)<<setprecision(6)<< duration<<" seconds"<<endl;//计算时间，使用时可以注释掉
}