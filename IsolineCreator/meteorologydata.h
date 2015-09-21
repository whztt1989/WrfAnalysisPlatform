#ifndef METEOROLOGYDATA_H
#define METEOROLOGYDATA_H

#include "isolinecreatorconfig.h"
#include <string>
/**
 * �洢�������ݵ���
 * @author wumin
 * @date 2012/09/09
 */
class EXPORT_CLASS MeteorologyData 
{
public:
	MeteorologyData();
	~MeteorologyData();

	/**
	 * ���ļ��ж�ȡ��������
	 * @return @c true if success
	 * @author wumin
	 * @date 2012/09/09
	 */
	bool load( std::string fileName );

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

	float* pValue; //����������

	float maxValue; //���ݵ����ֵ
	float minValue; //���ݵ���Сֵ
};

#endif