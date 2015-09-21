#ifndef ISOTOOLS_H
#define ISOTOOLS_H

#pragma once
#include <list>
#include "isolinecreatorconfig.h"

using namespace std;
namespace isotools
{
	/************************************************************************
				  *     y��
				        10s
						 -
						 -
						 -						 
	   OpenGL����ϵ,      0 ------ 10   x��
							
	************************************************************************/
	class EXPORT_CLASS Point2D 
	{
public:
		float x;//��������ֵ
		float y;//��������ֵ

		/***** ����� == ���� *********/
		bool operator == (Point2D A)
		{
			return A.x==this->x && A.y==this->y;
		}
	
	};
	
	/*****  ��ֵ�����ݽṹ������һ����ֵ�� *********/
	class EXPORT_CLASS Isoline
	{
public:
		float isovalue;//isoֵ
		bool isCircle;//�Ƿ񹹳ɻ�
		list<Point2D>  points;//�õ�ֵ�������еĵ�
		Point2D startPoint;//�õ�ֵ�� ͷ��� ���ڱߵ��е����������±�
		Point2D endPoint;//�õ�ֵ�� β��� ���ڱߵ��е����������±�
	};
}
#endif