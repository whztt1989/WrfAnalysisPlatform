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
	int year;//��
	int month;//��
	int day;//��
	int hour;//ʱ
	int lifecycle;//ʱЧ
	int level;//���
	float longitudeGridSpace;//���ȸ��
	float latitudeGridSpace;//γ�ȸ��
	float startLongitude;//��ʼ����
	float endLongitude;//��ֹ����
	float startLatitude;//��ʼγ��
	float endLatitude;//��ֹγ��
	int latitudeGridNumber;//γ����������һ��γ��Ȧ�Ͼ��ȸ������� 361
	int longitudeGridNumber;//���������������߷�����γ�ȸ�������  281
	float isolineSpace;//��ֵ�߼��������ֵ����ʼֵ����ֵֹ�ĵȲ����й���
	float isolineStartValue;//��Ҫ�����ĵ�ֵ����ʼֵ
	float isolineEndValue;//��Ҫ�����ĵ�ֵ����ֵֹ
	float smoothFactor;//ƽ��ϵ��(�ݲ�ʹ��)
	float bold;//�Ӵ���ֵ(�ݲ�ʹ��)

	float maxValue;//���ֵ ��Ҫ��
	float minValue;//��Сֵ ��Ҫ��
public:
	vector<vector<float> > value;
public:
	IsolineCreator(){};
	/************************************************************************/
	/* Funciton:   readGridData     
	 * Description: ������������
	 * Input:
	    startLongitude����ʼ����
		longitudeGridSpace�����ȸ��
		longitudeGridNumber�����������������߷�����γ�ȸ�������  281
		startLatitude����ʼγ��
		latitudeGridSpace��γ�ȸ��
		latitudeGridNumber��γ����������һ��γ��Ȧ�Ͼ��ȸ������� 361
		isolineStartValue����Ҫ�����ĵ�ֵ����ʼֵ
		isolineSpace����ֵ�߼��������ֵ����ʼֵ����ֵֹ�ĵȲ����й���
		data����������
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date�� 2012-8-30
	/************************************************************************/
	virtual bool readGridData(float startLongitude, float longitudeGridSpace, int longitudeGridNumber,float startLatitude,float latitudeGridSpace,int  latitudeGridNumber, float isolineStartValue,float isolineSpace, const float * data );
	/************************************************************************/
	/* Funciton:   generateContourLine     
	 * Description: ���������ļ�������Ч��Χ�ڵĵ�ֵ��ֵ����isoValues�У��������ɵĵ�ֵ�߸��������pathsPoints��
	 * Input:
		fileName: �����ļ���
		isoValues: ����Ҫ���Ƶĸ���ֵ�ߵ� isovalueֵ
		pathsPoints�� ��ά���飬�������ɵĲ�ͬ��ֵ���ϸ�������
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date�� 2012-8-24
	/************************************************************************/
	virtual void generateContourLine(vector<float> &isolineValueVector,vector< vector<isotools::Point2D> > &isolineList);

	/************************************************************************/
	/* Funciton:   generateContourLine     
	 * Description: ���������ļ�������Ч��Χ�ڵĵ�ֵ��ֵ����isoValues�У��������ɵĵ�ֵ�� ����pathsLinesV��
	 * Input:
		fileName: �����ļ���
		isoValues: ����Ҫ���Ƶĸ���ֵ�ߵ� isovalueֵ
		pathsLinesV�� ��ά���飬�������ɵĲ�ͬ��ֵ�ĵ�ֵ�߼��� 
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date�� 2012-9-11
	/************************************************************************/
	virtual void generateContourLine(vector<float> &isoValues,vector< vector<isotools::Isoline> > &pathsLinesV);
	/************************************************************************/
	/* Funciton:   generateContourLineWithIsovalue       
	 * Description: ����ָ���ĵ�ֵ��ֵ���ɵĵ�ֵ�ߣ���������������pPoints��
	 * Input:
		isovalue: ָ���ĵ�ֵ��ֵ
		pPoints�� �������ɵĵ�ֵ���ϸ�������
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date�� 2012-8-24
	/************************************************************************/
	virtual void generateContourLineWithIsovalue(float isovalue,vector<isotools::Point2D> &isoline );
	/************************************************************************/
	/* Funciton:   generateContourLineWithIsovalue       
	 * Description: ����ָ���ĵ�ֵ��ֵ���ɵĵ�ֵ�ߣ�����Isoline ���� pathsLines ��
	 * Input:
		isovalue: ָ���ĵ�ֵ��ֵ
		pathsLines�� �������ɵĵ�ֵ�� 
	 * Output: void
	 * Author: Wenshan Zhou
	 * Date�� 2012-9-11
	/************************************************************************/
	virtual void IsolineCreator::generateContourLineWithIsovalue(float isovalue,vector<isotools::Isoline> &pathsLines );
};