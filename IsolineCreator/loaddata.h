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
 * Description: 气候数据对应的类
 * Author: Wenshan Zhou
 * Date： 2012-8-24
/************************************************************************/
class  WeahterData 
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
		vector<vector<float> > value;

		float * pValue;
	WeahterData(){}
	/************************************************************************
	* Funciton:   load       
		* Description: 加载指定的气候文件数据至各成员变量中
		* Input:
		fileName: 气候文件名
		* Output: 加载成功返回 true ；失败返回false 
		* Author: Wenshan Zhou
		* Date： 2012-8-24
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
	//	getline(input,line);//第一行
	//	int pos = line.find("diamond");
	//	if (pos==string::npos )
	//	{
	//		cout<<"Data illegal! "<<endl;
	//		return false;
	//	}
	//	getline(input,line);//第二行
	//	int start =0,end =0,ti=0;
	//	float tmp[19];//将第二行数据暂存至此数组中
	//	//读参数
	//	do 
	//	{
	//		start = line.find_first_not_of(" ",start);
	//		end = line.find(" ",start);
	//		if (end==string::npos)//最后一个数据
	//		{
	//			tmp[ti++] = atof(line.substr(start ).c_str());
	//			break;
	//		}
	//		else
	//			tmp[ti++] = atof(line.substr(start,end-start).c_str());
	//		start =end+1;
	//	}while (start<line.size());
	//	//写入参数
	//	year = tmp[0];//年
	//	month = tmp[1]; //月
	//	day = tmp[2];//日
	//	hour = tmp[3];//时
	//	lifecycle = tmp[4];//时效
	//	level = tmp[5];//层次
	//	longitudeGridSpace = tmp[6];//经度格距
	//	latitudeGridSpace = tmp[7];//纬度格距
	//	startLongitude = tmp[8];//起始经度
	//	endLongitude = tmp[9];//终止经度
	//	startLatitude = tmp[10];//起始纬度
	//	endLatitude = tmp[11];//终止纬度
	//	latitudeGridNumber = tmp[12];//纬向格点数，即一个纬线圈上经度格点的数量 361 数组列数
	//	longitudeGridNumber = tmp[13];//经向格点数，即经线方向上纬度格点的数量  281 数组行数
	//	isolineSpace = tmp[14];//等值线间隔，即等值线起始值和终止值的等差数列公差
	//	isolineStartValue = tmp[15];//需要分析的等值线起始值
	//	isolineEndValue = tmp[16];//需要分析的等值线终止值
	//	smoothFactor = tmp[17];//平滑系数(暂不使用)
	//	bold = tmp[18];//加粗线值(暂不使用)

	//	pValue = new float[longitudeGridNumber*latitudeGridNumber];

	//	float *tmpPValue= pValue;
	//	value.resize(longitudeGridNumber);
	//	//初始化value
	//	for(int i =0;i <longitudeGridNumber ;++i)//逐行
	//	{
	//		value[i].resize(latitudeGridNumber);
	//	}
	//	maxValue = MIN_VALUE;
	//	minValue = MAX_VALUE;


	//	float tmpValue;
	//	//从第三行开始读入数据,
	//	for(int i =0;i < longitudeGridNumber;++i)//逐行
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
	//				if (end==string::npos && start==string::npos)//最后一行
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
		getline(input,line);//第一行
		int pos = line.find("diamond");
		if (pos==string::npos )
		{
			cout<<"Data illegal! "<<endl;
			return false;
		}
		getline(input,line);//第二行
		int start =0,end =0,ti=0;
		float tmp[19];//将第二行数据暂存至此数组中
		//读参数
		do 
		{
			start = line.find_first_not_of(" ",start);
			end = line.find(" ",start);
			if (end==string::npos)//最后一个数据
			{
				tmp[ti++] = atof(line.substr(start ).c_str());
				break;
			}
			else
				tmp[ti++] = atof(line.substr(start,end-start).c_str());
			start =end+1;
		}while (start<line.size());
		//写入参数
		year = tmp[0];//年
		month = tmp[1]; //月
		day = tmp[2];//日
		hour = tmp[3];//时
		lifecycle = tmp[4];//时效
		level = tmp[5];//层次
		longitudeGridSpace = tmp[6];//经度格距
		latitudeGridSpace = tmp[7];//纬度格距
		startLongitude = tmp[8];//起始经度
		endLongitude = tmp[9];//终止经度
		startLatitude = tmp[10];//起始纬度
		endLatitude = tmp[11];//终止纬度
		latitudeGridNumber = tmp[12];//纬向格点数，即一个纬线圈上经度格点的数量 361 数组列数
		longitudeGridNumber = tmp[13];//经向格点数，即经线方向上纬度格点的数量  281 数组行数
		isolineSpace = tmp[14];//等值线间隔，即等值线起始值和终止值的等差数列公差
		isolineStartValue = tmp[15];//需要分析的等值线起始值
		isolineEndValue = tmp[16];//需要分析的等值线终止值
		smoothFactor = tmp[17];//平滑系数(暂不使用)
		bold = tmp[18];//加粗线值(暂不使用)

		pValue = new float[longitudeGridNumber*latitudeGridNumber];

		float *tmpPValue= pValue;
		value.resize(longitudeGridNumber);
		//初始化value
		for(int i =0;i <longitudeGridNumber ;++i)//逐行
		{
			value[i].resize(latitudeGridNumber);
		}
		maxValue = MIN_VALUE;
		minValue = MAX_VALUE;


		float tmpValue;
		//从第三行开始读入数据,
		for(int i =0;i < longitudeGridNumber;++i)//逐行
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
					if (end==string::npos)//最后一行
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