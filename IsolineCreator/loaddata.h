#pragma once
#include<fstream>
#include<iostream>
#include<string>
#include<vector>
using namespace  std;

#define MAX_VALUE 10000000
#define MIN_VALUE -10000000


namespace loaddata
{
/************************************************************************/
/* class:   WeahterData     
 * Description: �������ݶ�Ӧ����
 * Author: Wenshan Zhou
 * Date�� 2012-8-24
/************************************************************************/
class  WeahterData 
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
		vector<vector<float> > value;

		float * pValue;
	WeahterData(){}
	/************************************************************************
	* Funciton:   load       
		* Description: ����ָ���������ļ�����������Ա������
		* Input:
		fileName: �����ļ���
		* Output: ���سɹ����� true ��ʧ�ܷ���false 
		* Author: Wenshan Zhou
		* Date�� 2012-8-24
	************************************************************************/
	//bool load(string fileName)
	//{
	//	ifstream input(fileName.c_str());
	//	if (input==NULL)
	//	{
	//		cout<<"Reading data failed! "<<endl;
	//		return false;
	//	}
	//	string line;
	//	getline(input,line);//��һ��
	//	int pos = line.find("diamond");
	//	if (pos==string::npos )
	//	{
	//		cout<<"Data illegal! "<<endl;
	//		return false;
	//	}
	//	getline(input,line);//�ڶ���
	//	int start =0,end =0,ti=0;
	//	float tmp[19];//���ڶ��������ݴ�����������
	//	//������
	//	do 
	//	{
	//		start = line.find_first_not_of(" ",start);
	//		end = line.find(" ",start);
	//		if (end==string::npos)//���һ������
	//		{
	//			tmp[ti++] = atof(line.substr(start ).c_str());
	//			break;
	//		}
	//		else
	//			tmp[ti++] = atof(line.substr(start,end-start).c_str());
	//		start =end+1;
	//	}while (start<line.size());
	//	//д�����
	//	year = tmp[0];//��
	//	month = tmp[1]; //��
	//	day = tmp[2];//��
	//	hour = tmp[3];//ʱ
	//	lifecycle = tmp[4];//ʱЧ
	//	level = tmp[5];//���
	//	longitudeGridSpace = tmp[6];//���ȸ��
	//	latitudeGridSpace = tmp[7];//γ�ȸ��
	//	startLongitude = tmp[8];//��ʼ����
	//	endLongitude = tmp[9];//��ֹ����
	//	startLatitude = tmp[10];//��ʼγ��
	//	endLatitude = tmp[11];//��ֹγ��
	//	latitudeGridNumber = tmp[12];//γ����������һ��γ��Ȧ�Ͼ��ȸ������� 361 ��������
	//	longitudeGridNumber = tmp[13];//���������������߷�����γ�ȸ�������  281 ��������
	//	isolineSpace = tmp[14];//��ֵ�߼��������ֵ����ʼֵ����ֵֹ�ĵȲ����й���
	//	isolineStartValue = tmp[15];//��Ҫ�����ĵ�ֵ����ʼֵ
	//	isolineEndValue = tmp[16];//��Ҫ�����ĵ�ֵ����ֵֹ
	//	smoothFactor = tmp[17];//ƽ��ϵ��(�ݲ�ʹ��)
	//	bold = tmp[18];//�Ӵ���ֵ(�ݲ�ʹ��)

	//	pValue = new float[longitudeGridNumber*latitudeGridNumber];

	//	float *tmpPValue= pValue;
	//	value.resize(longitudeGridNumber);
	//	//��ʼ��value
	//	for(int i =0;i <longitudeGridNumber ;++i)//����
	//	{
	//		value[i].resize(latitudeGridNumber);
	//	}
	//	maxValue = MIN_VALUE;
	//	minValue = MAX_VALUE;


	//	float tmpValue;
	//	//�ӵ����п�ʼ��������,
	//	for(int i =0;i < longitudeGridNumber;++i)//����
	//	{
	//		int j=0;
	//		while (j< latitudeGridNumber)
	//		{
	//			getline(input,line);
	//			int start=0,end =0;
	//			do 
	//			{
	//				start = line.find_first_not_of(" ",start);
	//				end = line.find(" ",start);
	//				if (end==string::npos && start==string::npos)//���һ��
	//				{
	//					//tmpValue  = atof(line.substr(start ).c_str());
	//					//
	//					//if(maxValue < tmpValue) maxValue = tmpValue;
	//					//if(minValue > tmpValue) minValue = tmpValue;

	//					//value[i][j++] = tmpValue;
	//					//*(tmpPValue++)=tmpValue;
	//					break;
	//				}
	//				else
	//				{
	//					if (end==string::npos)
	//						end=line.size();
	//			    		tmpValue = atof(line.substr(start,end-start).c_str());
	//						if(maxValue < tmpValue) maxValue = tmpValue;
	//						if(minValue > tmpValue) minValue = tmpValue;

	//						value[i][j++] = tmpValue;
	//						*(tmpPValue++)=tmpValue;
	//				}
	//				start =end+1;
	//			}while (start<line.size());
	//		}
	//		
	//	}
	//	return true;
	//}

	bool load(string fileName)
	{
		ifstream input(fileName.c_str());
		if (input==NULL)
		{
			cout<<"Reading data failed! "<<endl;
			return false;
		}
		string line;
		getline(input,line);//��һ��
		int pos = line.find("diamond");
		if (pos==string::npos )
		{
			cout<<"Data illegal! "<<endl;
			return false;
		}
		getline(input,line);//�ڶ���
		int start =0,end =0,ti=0;
		float tmp[19];//���ڶ��������ݴ�����������
		//������
		do 
		{
			start = line.find_first_not_of(" ",start);
			end = line.find(" ",start);
			if (end==string::npos)//���һ������
			{
				tmp[ti++] = atof(line.substr(start ).c_str());
				break;
			}
			else
				tmp[ti++] = atof(line.substr(start,end-start).c_str());
			start =end+1;
		}while (start<line.size());
		//д�����
		year = tmp[0];//��
		month = tmp[1]; //��
		day = tmp[2];//��
		hour = tmp[3];//ʱ
		lifecycle = tmp[4];//ʱЧ
		level = tmp[5];//���
		longitudeGridSpace = tmp[6];//���ȸ��
		latitudeGridSpace = tmp[7];//γ�ȸ��
		startLongitude = tmp[8];//��ʼ����
		endLongitude = tmp[9];//��ֹ����
		startLatitude = tmp[10];//��ʼγ��
		endLatitude = tmp[11];//��ֹγ��
		latitudeGridNumber = tmp[12];//γ����������һ��γ��Ȧ�Ͼ��ȸ������� 361 ��������
		longitudeGridNumber = tmp[13];//���������������߷�����γ�ȸ�������  281 ��������
		isolineSpace = tmp[14];//��ֵ�߼��������ֵ����ʼֵ����ֵֹ�ĵȲ����й���
		isolineStartValue = tmp[15];//��Ҫ�����ĵ�ֵ����ʼֵ
		isolineEndValue = tmp[16];//��Ҫ�����ĵ�ֵ����ֵֹ
		smoothFactor = tmp[17];//ƽ��ϵ��(�ݲ�ʹ��)
		bold = tmp[18];//�Ӵ���ֵ(�ݲ�ʹ��)

		pValue = new float[longitudeGridNumber*latitudeGridNumber];

		float *tmpPValue= pValue;
		value.resize(longitudeGridNumber);
		//��ʼ��value
		for(int i =0;i <longitudeGridNumber ;++i)//����
		{
			value[i].resize(latitudeGridNumber);
		}
		maxValue = MIN_VALUE;
		minValue = MAX_VALUE;


		float tmpValue;
		//�ӵ����п�ʼ��������,
		for(int i =0;i < longitudeGridNumber;++i)//����
		{
			int j=0;
			while (j< latitudeGridNumber)
			{
				getline(input,line);
				int start=0,end =0;
				do 
				{
					start = line.find_first_not_of(" ",start);
					end = line.find(" ",start);
					if (end==string::npos)//���һ��
					{
						tmpValue  = atof(line.substr(start ).c_str());
						
						if(maxValue < tmpValue) maxValue = tmpValue;
						if(minValue > tmpValue) minValue = tmpValue;

						value[i][j++] = tmpValue;
						*(tmpPValue++)=tmpValue;
						break;
					}
					else
					{
				    		tmpValue = atof(line.substr(start,end-start).c_str());
							if(maxValue < tmpValue) maxValue = tmpValue;
							if(minValue > tmpValue) minValue = tmpValue;

							value[i][j++] = tmpValue;
							*(tmpPValue++)=tmpValue;
					}
					start =end+1;
				}while (start<line.size());
			}
			
		}
		return true;
	}
	};
	
	
}