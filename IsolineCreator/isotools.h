#ifndef ISOTOOLS_H
#define ISOTOOLS_H

#pragma once
#include <list>
#include "isolinecreatorconfig.h"

using namespace std;
namespace isotools
{
	/************************************************************************
				  *     y轴
				        10s
						 -
						 -
						 -						 
	   OpenGL坐标系,      0 ------ 10   x轴
							
	************************************************************************/
	class EXPORT_CLASS Point2D 
	{
public:
		float x;//横轴坐标值
		float y;//纵轴坐标值

		/***** 运算符 == 重载 *********/
		bool operator == (Point2D A)
		{
			return A.x==this->x && A.y==this->y;
		}
	
	};
	
	/*****  等值线数据结构，保存一条等值线 *********/
	class EXPORT_CLASS Isoline
	{
public:
		float isovalue;//iso值
		bool isCircle;//是否构成环
		list<Point2D>  points;//该等值线上所有的点
		Point2D startPoint;//该等值线 头结点 所在边的中点数组索引下标
		Point2D endPoint;//该等值线 尾结点 所在边的中点数组索引下标
	};
}
#endif