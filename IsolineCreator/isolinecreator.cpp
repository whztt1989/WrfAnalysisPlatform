#include "isolinecreator.h"
#include "loaddata.h"
#include<ctime>
#include<iomanip>
#include "marchingsquares.h"


bool IsolineCreator::readGridData(float startLongitude, float longitudeGridSpace, int longitudeGridNumber,float startLatitude,float latitudeGridSpace,int  latitudeGridNumber, float isolineStartValue,float isolineSpace, const float * data )
{
	this->startLongitude = startLongitude; //��ʼ����
	this->longitudeGridSpace = longitudeGridSpace; //���ȸ��
	this->longitudeGridNumber = longitudeGridNumber; //���������������߷�����γ�ȸ�������  281
	this->endLongitude = this->startLongitude + (latitudeGridNumber-1)*longitudeGridSpace;

	this->startLatitude = startLatitude;//��ʼγ��
	this->latitudeGridSpace = latitudeGridSpace;//γ�ȸ��
	this->latitudeGridNumber = latitudeGridNumber;//γ����������һ��γ��Ȧ�Ͼ��ȸ������� 361
	this->endLatitude = this->startLatitude + (longitudeGridNumber-1)*latitudeGridSpace;

	this->isolineStartValue = isolineStartValue;
	this->isolineSpace = isolineSpace;
	
	maxValue = MIN_VALUE;
	minValue = MAX_VALUE;

	this->value.resize(longitudeGridNumber);
	//��ʼ��value
	for(int i =0;i <longitudeGridNumber ;++i)//����
	{
		this->value[i].resize(latitudeGridNumber);
	}
	float tmpValue =0;
	for(int i =0;i < longitudeGridNumber;++i)//����
		for (int j=0;j<latitudeGridNumber;++j)//����
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

	//����ʱ��
	//clock_t start,finish;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//double duration;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//cout<<"time begin to count:"<<endl;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//start = clock();//����ʱ�䣬ʹ��ʱ����ע�͵�

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
	//finish = clock();//����ʱ�䣬ʹ��ʱ����ע�͵�
	//duration = (double)(finish-start)/CLOCKS_PER_SEC;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//cout<<"the total cost time is: "<<setiosflags(ios::fixed)<<setprecision(6)<< duration<<" seconds"<<endl;//����ʱ�䣬ʹ��ʱ����ע�͵�
}

void IsolineCreator::generateContourLine(vector<float> &isoValues,vector< vector<isotools::Isoline> > &pathsLinesV)
{

	//����ʱ��
	//clock_t start,finish;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//double duration;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//cout<<"time begin to count:"<<endl;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//start = clock();//����ʱ�䣬ʹ��ʱ����ע�͵�

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
	//����һ��
	//for (int i=0;i<isoValues.size();++i)
	//{
	//	vector<isotools::Isoline> pLines;
	//	marchingsquares::doMarchingSquares(value,isoValues[i],pLines,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	//	pathsLines.push_back(pLines);
	//}
	//��������
	 marchingsquares::doMarchingSquares(value,isoValues,pathsLinesV,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	//finish = clock();//����ʱ�䣬ʹ��ʱ����ע�͵�
	//duration = (double)(finish-start)/CLOCKS_PER_SEC;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//cout<<"the total cost time is: "<<setiosflags(ios::fixed)<<setprecision(6)<< duration<<" seconds"<<endl;//����ʱ�䣬ʹ��ʱ����ע�͵�
}

void IsolineCreator::generateContourLineWithIsovalue(float isovalue,vector<isotools::Point2D> &pPoints )
{
	//clock_t start,finish; //����ʱ�䣬ʹ��ʱ����ע�͵�
	//double duration;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//cout<<"time begin to count:"<<endl;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//start = clock();//����ʱ�䣬ʹ��ʱ����ע�͵�
	//vector<MarchingSquares::Point2D> pPoints;
	marchingsquares::doMarchingSquares(value,isovalue,pPoints,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	//finish = clock();//����ʱ�䣬ʹ��ʱ����ע�͵�
	//duration = (double)(finish-start)/CLOCKS_PER_SEC;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//cout<<"the total cost time is: "<<setiosflags(ios::fixed)<<setprecision(6)<< duration<<" seconds"<<endl;//����ʱ�䣬ʹ��ʱ����ע�͵�
}

void IsolineCreator::generateContourLineWithIsovalue(float isovalue,vector<isotools::Isoline> &pathsLines )
{
	//clock_t start,finish; //����ʱ�䣬ʹ��ʱ����ע�͵�
	//double duration;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//cout<<"time begin to count:"<<endl;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//start = clock();//����ʱ�䣬ʹ��ʱ����ע�͵�
	//vector<MarchingSquares::Point2D> pPoints;
	marchingsquares::doMarchingSquares(value,isovalue,pathsLines,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	//finish = clock();//����ʱ�䣬ʹ��ʱ����ע�͵�
	//duration = (double)(finish-start)/CLOCKS_PER_SEC;//����ʱ�䣬ʹ��ʱ����ע�͵�
	//cout<<"the total cost time is: "<<setiosflags(ios::fixed)<<setprecision(6)<< duration<<" seconds"<<endl;//����ʱ�䣬ʹ��ʱ����ע�͵�
}