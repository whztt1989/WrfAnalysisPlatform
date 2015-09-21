#pragma once
#include<iostream>
#include<ctime>
#include<vector>
#include "isotools.h"

using namespace std;
/************************************************************************/
/* namespace:   MarchingSquares     
 * Description: 提供使用MarchingSquares方法生成各等值线坐标值的方法接口
 * Author: Wenshan Zhou
 * Date： 2012-8-24
/************************************************************************/
namespace marchingsquares
{

 // note whether the  edge has cut node (1 Yes, 0 No) 
/*    1     edge NO.
     ___
  0 |   | 2
     ---
	  3
*/   
const int EdgeTable[16] = {
            0x0,     //0000,
            0x9,     //1001,
            0x3,     //0011
            0xa,     //1010

            0x6,     //0110, 
            0xf,     //1111,//
            0x5,     //0101
            0xc,     //1100

            0xc,     //1100
            0x5,     //0101
            0xf,     //1111,
            0x6,     //0110,

            0xa,     //1010
            0x3,     //0011
            0x9,     //1001,
            0x0,     //0000
        };
 // note the sub array is length 5, with a -1 at the end to mark end of array 与上面的EdgeTable[]值一一对应
const int SegmentTable[16][5] = {

            {-1,-1,-1,-1,-1},
            {0,3,-1,-1,-1},
            {2,3,-1,-1,-1},
            {0,2,-1,-1,-1},

            {1,2,-1,-1,-1},
            {0,1,2,3,-1}, //index =5 ,  0-1  2-3 
            {1,3,-1,-1,-1},
            {0,1,-1,-1,-1},

            {0,1,-1,-1,-1},
            {1,3,-1,-1,-1},
            {0,3,1,2,-1},//index =10 ,  0-3  1-2  
            {1,2,-1,-1,-1},

            {0,2,-1,-1,-1},
            {2,3,-1,-1,-1},
			{3,0,-1,-1,-1},
            {-1,-1,-1,-1,-1}
        };


const float V_EPSILON = 1.0f / 256.0f;//误差阈值

/************************************************************************/
/* Funciton:   VertexInterp     
 * Description: 插值函数
 * Input:
	isovalue: 等值线值
	x1： 第一个顶点的数组 x 下标
	y1： 第一个顶点的数组 y 下标
	v1:  第一个顶点值
	x2： 第二个顶点的数组 x 下标
	y2： 第二个顶点的数组 y 下标
	v2:  第二个顶点值
 * Output: 包含插值点的坐标及等值线值的Point2D对象
 * Author: Wenshan Zhou
 * Date： 2012-8-24
 * Modified : 2012-9-9 by Wenshan Zhou
************************************************************************/
static isotools::Point2D  VertexInterp( float isovalue, int x1,int y1,float v1, int x2,int y2,float v2,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
{
        float mu;
        isotools::Point2D p;

        if ( std::abs(v1-v2) < V_EPSILON )
		{
			p.x = y1 * longitudeGridSpace + startLongitude;//由数组下标换成OpenGL坐标系
			p.y = x1 * latitudeGridSpace + startLatitude; //由数组下标换成OpenGL坐标系
            return p;
		}

        mu = (isovalue - v1) / (v2 - v1);
		p.x = (y1 + mu * (float)(y2 - y1)) * longitudeGridSpace + startLongitude;//由数组下标换成OpenGL坐标系
		p.y = (x1 + mu * (float)(x2 - x1)) * latitudeGridSpace + startLatitude;//由数组下标换成OpenGL坐标系
		
        return p;
 }
 
 /************************************************************************/
/* Funciton:   getMiddlePoint     
 * Description: 获得交点所在边的中点（其坐标相当于该边的全局编号）
 * Input:
	edgeIndex: 交点所在边在cell中的局部编号（0,1,2,3）
	i： 交点所在cell中左上角点的数组x值下标
	j：交点所在cell中左上角点的数组y值下标
 * Output: isotools::Point2D  返回该边的中点
 * Author: Wenshan Zhou
 * Date： 2012-9-9
/************************************************************************/
 static isotools::Point2D getMiddlePoint(int edgeIndex,int i,int j)
 {
	 isotools::Point2D p;
	 switch (edgeIndex)
	 {
	 case 0:
		 p.x = i+0.5;p.y=j;break;
	 case 1:
		 p.x = i;p.y =j+0.5;break;
	 case 2:
		 p.x = i+0.5;p.y =j+1;break;
	 case 3:
		 p.x = i+1;p.y =j+0.5;break;
	 }
	 return p;
 }

 /************************************************************************/
/* Funciton:   getCutPoint     
 * Description: 通过插值获得交点的坐标
 * Input:
	edgeIndex: 交点所在边在cell中的局部编号（0,1,2,3）
	i： 交点所在cell中左上角点的数组x值下标
	j：交点所在cell中左上角点的数组y值下标
	data：天气数据值二维数组
	isovalue：等值
 * Output: isotools::Point2D  返回该交点的坐标
 * Author: Wenshan Zhou
 * Date： 2012-9-10
/************************************************************************/
 static isotools::Point2D getCutPoint(int edgeIndex,int i,int j,vector<vector<float> > &data,float isovalue,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 isotools::Point2D p;
	 switch(edgeIndex)
	 {
	 case 0:
		 return VertexInterp(isovalue,i+1,j,data[i+1][j],i,j,data[i][j],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace); //与node 编号有关
	 case 1:
		 return VertexInterp(isovalue,i,j,data[i][j],i,j+1,data[i][j+1],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	 case 2:
		  return VertexInterp(isovalue,i,j+1,data[i][j+1],i+1,j+1,data[i+1][j+1],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	 case 3:
		  return VertexInterp(isovalue,i+1,j+1,data[i+1][j+1],i+1,j,data[i+1][j],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
	 }

 }
 /************************************************************************/
 /* Funciton:   addPointToLine    
  * Description: 进行线段合并操作，若有线段合并，则返回true，否则返回flase
  * type 0:表示是头结点 ,1表示尾节点
 */
 static bool isMergeIsoLine( isotools::Point2D mid,int type,vector<isotools::Isoline> &pathLines,int m )
 {
	 for (int i=0;i<pathLines.size();++i)
	 {
		 if(i==m)continue;

		 if (mid == pathLines[i].startPoint && type ==0)//两条线的 头结点 重合，则将短的线段加到长的线段，并删除短的线段
		 {
			 if(pathLines[m].points.size() > pathLines[i].points.size())//线段m较长，将i中的点移动到m中
			 {
				 list<isotools::Point2D>::iterator pIt = pathLines[i].points.begin();//指向头结点
				 pIt++;//跳过头结点
				 isotools::Point2D point ;
				 while (pIt!=pathLines[i].points.end())
				 {
					 point= *pIt;//取值
					 pathLines[m].points.push_front(point);//在头部依次插入
					 pIt++;
				 }
				  //修改m的头结点
				  pathLines[m].startPoint = pathLines[i].endPoint;

				 //删除i
				  vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
				  int k=0;
				  while (k!=i)//移动迭代器至线段pathLines[i]处
				  {
					  lineIt++;
					  k++;
				  }
				  pathLines.erase(lineIt);//删除线段pathLines[i]
				 

			 }
			 else //线段i较长，将m中的点移动到i中
			 {
				 list<isotools::Point2D>::iterator pIt = pathLines[m].points.begin();//指向头结点
				 pIt++;//跳过头结点
				 isotools::Point2D point ;
				 while (pIt!=pathLines[m].points.end())
				 {
					 point= *pIt;//取值
					 pathLines[i].points.push_front(point);//在头部依次插入
					 pIt++;
				 }
				 //修改i的头结点
				 pathLines[i].startPoint = pathLines[m].endPoint;
				
				 //删除m
				 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
				 int k=0;
				 while (k!=m)//移动迭代器至线段pathLines[m]处
				 {
					  k++;
					 lineIt++;
				 }
				 pathLines.erase(lineIt);//删除线段pathLines[m]
			 }
			 return true;
		 }
		 else if (mid == pathLines[i].endPoint && type ==1)//两条线的 尾结点 重合，则将短的线段加到长的线段，并删除短的线段
		 {
			 if(pathLines[m].points.size() > pathLines[i].points.size())//线段m较长，将i中的点移动到m中
			 {
				 list<isotools::Point2D>::iterator pIt = pathLines[i].points.end();//指向尾结点
				 pIt--;//跳过end
				 pIt--;//跳过尾结点
				 isotools::Point2D point ;
				 while (pIt!=pathLines[i].points.begin())
				 {
					 point= *pIt;//取值
					 pathLines[m].points.push_back(point);//在尾部依次插入
					 pIt--;
				 }
				 //再push头结点
				 point= *pIt;//取值
				 pathLines[m].points.push_back(point);//在尾部插入
				 //修改m的尾结点
				 pathLines[m].endPoint = pathLines[i].startPoint;

				 //删除i
				 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
				 int k=0;
				 while (k!=i)//移动迭代器至线段pathLines[i]处
				 {
					  k++;
					 lineIt++;
				 }
				 pathLines.erase(lineIt);//删除线段pathLines[i]

			 }
			 else //线段i较长，将m中的点移动到i中
			 {
				 list<isotools::Point2D>::iterator pIt = pathLines[m].points.end();//指向尾结点
				 pIt--;//跳过end
				 pIt--;//跳过尾结点
				 isotools::Point2D point ;
				 while (pIt!=pathLines[m].points.begin())
				 {
					 point= *pIt;//取值
					 pathLines[i].points.push_back(point);//在尾部依次插入
					 pIt--;
				 }
				 //再push头结点
				 point= *pIt;//取值
				 pathLines[i].points.push_back(point);//在尾部插入
				 //修改i的尾结点
				 pathLines[i].endPoint = pathLines[m].startPoint;
				
				 //删除m
				 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
				 int k=0;
				 while (k!=m)//移动迭代器至线段pathLines[m]处
				 {
					  k++;
					 lineIt++;
				 }
				 pathLines.erase(lineIt);//删除线段pathLines[m]
			 }
			  return true;
		 }
		 else if (mid == pathLines[i].endPoint && type ==0)//m线的头结点与i的尾结点 重合，则将m加到i后
		 {
			 //将m中的点移动到i中
			 list<isotools::Point2D>::iterator pIt = pathLines[m].points.begin();//指向头结点
			 pIt++;//跳过头结点
			 isotools::Point2D point ;
			 while (pIt!=pathLines[m].points.end())
			 {
				 point= *pIt;//取值
				 pathLines[i].points.push_back(point);//在尾部依次插入
				 pIt++;
			 }
			 //修改i的尾结点
			 pathLines[i].endPoint = pathLines[m].endPoint;

			 //删除m
			 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
			 int k=0;
			 while (k!=m)//移动迭代器至线段pathLines[m]处
			 {
				  k++;
				 lineIt++;
			 }
			 pathLines.erase(lineIt);//删除线段pathLines[m]
			 return true;
		 }
		 else if (mid == pathLines[i].startPoint && type ==1)//m线的尾结点与i的头结点 重合，则将i加到m后
		 {
			 //将m中的点移动到i中
			 list<isotools::Point2D>::iterator pIt = pathLines[i].points.begin();//指向头结点
			 pIt++;//跳过头结点
			 isotools::Point2D point ;
			 while (pIt!=pathLines[i].points.end())
			 {
				 point= *pIt;//取值
				 pathLines[m].points.push_back(point);//在尾部依次插入
				 pIt++;
			 }
			 //修改m的头结点
			 pathLines[m].endPoint = pathLines[i].endPoint;

			 //删除i
			 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
			 int k=0;
			 while (k!=i)//移动迭代器至线段pathLines[i]处
			 {
				  k++;
				 lineIt++;
			 }
			 pathLines.erase(lineIt);//删除线段pathLines[i]
			 return true;
		 }
	 }
	 return false;
 }

 /************************************************************************/
/* Funciton:   addPointToLine     
 * Description:  判断当前两边上的点是否与已有等值线的首尾点重合，若是，则忽略此重合点，再将另一个点插入到首或尾并判断加入后是否与已有其他线段端点重合，以进行合并操作；否则，新增一条等值线
 * Input:
	edgeIndex1: 交点1所在边在cell中的局部编号（0,1,2,3）
	edgeIndex2: 交点2所在边在cell中的局部编号（0,1,2,3） （这两个交点构成了cell中的一条等值线）
	i： 交点所在cell中左上角点的数组x值下标
	j：交点所在cell中左上角点的数组y值下标
	data：天气数据值二维数组
	isovalue：等值
	pathLines：返回该等值对应的各条等值线数组 （每条保存在一个Isoline）
 * Output: isotools::Point2D  返回该边的中点
 * Author: Wenshan Zhou
 * Date： 2012-9-9
/************************************************************************/
 static void  addPointToLine(int edgeIndex1,int edgeIndex2 , int i,int j,vector<vector<float> > &data,float isovalue,vector<isotools::Isoline> &pathLines,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 //判断当前边上的点是否与已有等值线的首尾点重合，若是，则忽略此重合点，再将另一个点插入到首或尾，否则，新增一条等值线
	
	  isotools::Point2D mid1 = getMiddlePoint(edgeIndex1,i,j);//存放数组索引
	  isotools::Point2D mid2 = getMiddlePoint(edgeIndex2,i,j);//存放数组索引

	  int m;
	  for (m= 0;m<pathLines.size();++m)
	  {
		  if (pathLines[m].isCircle)
		  {
			  continue;//对于构成环的不予判断
		  }
		  //若边上的点与首尾结点都相同
		  if ((mid1 == pathLines[m].startPoint && mid2 == pathLines[m].endPoint)||(mid2 == pathLines[m].startPoint && mid1 == pathLines[m].endPoint))
		  {
			  //isotools::Point2D point = pathLines[m].points.front();//返回第一个元素  【不再多插一个元素,而是只改isCircle标记
			  //pathLines[m].points.push_back(point);//多插入第一个元素至末尾，形成回路  【如点1,2,3构成回路，则只保存点1,2,3，不再保存1,2,3,1】
			  pathLines[m].isCircle = true;//形成回路
			  break;
		  } 
		  else if (mid1 == pathLines[m].startPoint )//若m1与头结点重合
		  {//则在头结点前插入另一个点，并更新头结点
			  isotools::Point2D point = getCutPoint(edgeIndex2,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			  pathLines[m].points.push_front(point);
			  pathLines[m].startPoint = mid2;
			  isMergeIsoLine(mid2,0,pathLines,m);//进行合并操作
			  break;
		  }
		  else if (mid1 == pathLines[m].endPoint)//若m1与尾结点重合
		  {
			  isotools::Point2D point = getCutPoint(edgeIndex2,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			  pathLines[m].points.push_back(point);
			  pathLines[m].endPoint = mid2;
			  isMergeIsoLine(mid2,1,pathLines,m);//进行合并操作
			  break;
		  }
		  else if (mid2 == pathLines[m].startPoint)//若m2与头结点重合
		  { //则在头结点前插入另一个点，并更新头结点
			  isotools::Point2D point = getCutPoint(edgeIndex1,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			  pathLines[m].points.push_front(point);
			  pathLines[m].startPoint = mid1;
			  isMergeIsoLine(mid1,0,pathLines,m);//进行合并操作
			  break;
		  }
		  else if (mid2 == pathLines[m].endPoint)//若m2与尾结点重合
		  {
			  isotools::Point2D point = getCutPoint(edgeIndex1,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			  pathLines[m].points.push_back(point);
			  pathLines[m].endPoint = mid1;
			  isMergeIsoLine(mid1,1,pathLines,m);//进行合并操作
			  break;
		  }
	  }
	  if(m == pathLines.size())//说明这条边没有与任何一条等值线相接，则新创建一条等值线
	  {
		  isotools::Point2D point1 = getCutPoint(edgeIndex1,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
		  isotools::Point2D point2 = getCutPoint(edgeIndex2,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
		  isotools::Isoline isoList;
		  isoList.points.push_front(point1);
		  isoList.startPoint = mid1;
		  isoList.points.push_back(point2);
		  isoList.endPoint = mid2;
		  isoList.isovalue = isovalue;
		  isoList.isCircle = false;
		  pathLines.push_back(isoList);//新增一条等值线
	  }
 }

 /************************************************************************/
/* Funciton:   doMarchingSquares     【此方法便于用 GPU 并行加速】
 * Description: Marching Squares 算法的实现
 * Input:
	data: 天气数据值二维数组
	isovalue： 等值线值
	pathPoints： 生成的等值线坐标
 * Output: void
 * Author: Wenshan Zhou
 * Date： 2012-8-23
 * Modified: 2012-9-9 by Wenshan Zhou  
/************************************************************************/
 static void   doMarchingSquares(vector<vector<float> > &data,float isovalue,vector<isotools::Point2D> &pathPoints,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 pathPoints.clear();
	 for(int i =0; i<data.size()-1;++i)//逐行扫
	 {
		 for(int j=0;j<data[i].size()-1;++j)//逐列扫
		 {
			 int squareIndex = 0;

			 if(data[i][j] > isovalue) squareIndex|=8;
			 if(data[i][j+1] > isovalue) squareIndex|=4;
			 if(data[i+1][j+1] > isovalue) squareIndex|=2;
			 if(data[i+1][j] > isovalue) squareIndex|=1;


			 if(EdgeTable[squareIndex]==0 )
				 continue;

			 //消除二义性
			 if (squareIndex == 5)// index = 0x5 
			 {
				 float centerValue =(1/4.0) *(data[i][j]+data[i][j+1]+data[i+1][j+1]+data[i+1][j]);
				 if (centerValue < isovalue)
				 {
					 squareIndex=10;
				 }
			 }
			 else if(squareIndex == 10)//0x10
			 {
				 float centerValue =(1/4.0) *(data[i][j]+data[i][j+1]+data[i+1][j+1]+data[i+1][j]);
				 if (centerValue < isovalue)
				 {
					 squareIndex=5;
				 }
			 }

			isotools::Point2D vertlist[4];//最多每边上各一个交点，共四个

			 if (EdgeTable[squareIndex] & 8)//0号边上有点 
			 {
				 vertlist[0] = VertexInterp(isovalue,i+1,j,data[i+1][j],i,j,data[i][j],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace); //与node 编号有关
			 }

			 if (EdgeTable[squareIndex] & 4)//1号边上有点
			 {
				 vertlist[1] = VertexInterp(isovalue,i,j,data[i][j],i,j+1,data[i][j+1],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			 }

			 if (EdgeTable[squareIndex] & 2)//2号边上有点
			 {
				 vertlist[2] = VertexInterp(isovalue,i,j+1,data[i][j+1],i+1,j+1,data[i+1][j+1],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			 }

			 if (EdgeTable[squareIndex] & 1)//3号边上有点
			 {
				 vertlist[3] = VertexInterp(isovalue,i+1,j+1,data[i+1][j+1],i+1,j,data[i+1][j],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			 }
			 //collect the pathPoints,两两输出一段
			 for(int k=0;SegmentTable[squareIndex][k] !=-1;k=k+2)
			 {
				 pathPoints.push_back(vertlist[SegmentTable[squareIndex][k]]);
				 pathPoints.push_back(vertlist[SegmentTable[squareIndex][k+1]]);
			 }


		 }
	 }
 }
 /************************************************************************/
/* Funciton:   doMarchingSquares      【此方法适用于 CPU】
 * Description: Marching Squares 算法的实现
 * Input:
	data: 天气数据值二维数组
	isovalue： 等值线值
	pathLines： 返回该等值下的各条等值线数组
 * Output: void
 * Author: Wenshan Zhou
 * Date： 2012-9-10
/************************************************************************/
 static void   doMarchingSquares(vector<vector<float> > &data,float isovalue,vector<isotools::Isoline> &pathLines,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 pathLines.clear();
	 for(int i =0; i<data.size()-1;++i)//逐行扫
	 {
		 for(int j=0;j<data[i].size()-1;++j)//逐列扫
		 {
			 int squareIndex = 0;

			 if(data[i][j] > isovalue) squareIndex|=8;
			 if(data[i][j+1] > isovalue) squareIndex|=4;
			 if(data[i+1][j+1] > isovalue) squareIndex|=2;
			 if(data[i+1][j] > isovalue) squareIndex|=1;


			 if(EdgeTable[squareIndex]==0 )
				 continue;

			 //消除二义性
			 if (squareIndex == 5)// index = 0x5 
			 {
				 float centerValue =(1/4.0) *(data[i][j]+data[i][j+1]+data[i+1][j+1]+data[i+1][j]);
				 if (centerValue < isovalue)
				 {
					 squareIndex=10;
				 }
			 }
			 else if(squareIndex == 10)//0x10
			 {
				 float centerValue =(1/4.0) *(data[i][j]+data[i][j+1]+data[i+1][j+1]+data[i+1][j]);
				 if (centerValue < isovalue)
				 {
					 squareIndex=5;
				 }
			 }

			 int edgeIndex1=0,edgeIndex2=0;

			 //collect the pathPoints,两两输出一段
			 for(int k=0;SegmentTable[squareIndex][k] !=-1;k=k+2)
			 {
				 edgeIndex1 = SegmentTable[squareIndex][k];
				 edgeIndex2 = SegmentTable[squareIndex][k+1];
				 addPointToLine(edgeIndex1,edgeIndex2,i,j,data,isovalue,pathLines,startLongitude,longitudeGridSpace, startLatitude,latitudeGridSpace);
			 }	
		 }
	 }
 }
 
 /************************************************************************/
/* Funciton:   doMarchingSquares      【此方法适用于 CPU】 
 * Description: Marching Squares 算法的实现
 * Input:
	data: 天气数据值二维数组
	isovalues： 等值线值数组
	pathLinesV： 返回该等值数组下的所有各条等值线数组的集合
 * Output: void
 * Author: Wenshan Zhou
 * Date： 2012-9-10
/************************************************************************/
 static void   doMarchingSquares(vector<vector<float> > &data,vector<float> &isovalues,vector< vector<isotools::Isoline>> &pathLinesV,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 pathLinesV.clear();
	 vector<int> squareIndexs;
	 squareIndexs.resize(isovalues.size());
	 pathLinesV.resize(isovalues.size());

	 for(int i =0; i<data.size()-1;++i)//逐行扫
	 {
		 for(int j=0;j<data[i].size()-1;++j)//逐列扫
		 {
			 for (int m=0;m<isovalues.size();++m)
			 {
				 squareIndexs[m]=0;
				 if(data[i][j] > isovalues[m]) squareIndexs[m]|=8;
				 if(data[i][j+1] > isovalues[m]) squareIndexs[m]|=4;
				 if(data[i+1][j+1] > isovalues[m]) squareIndexs[m]|=2;
				 if(data[i+1][j] > isovalues[m]) squareIndexs[m]|=1;

				 if (EdgeTable[squareIndexs[m]]!=0)
				 {
					 //消除二义性
					 if (squareIndexs[m] == 5)// index = 0x5 
					 {
						 float centerValue =(1/4.0) *(data[i][j]+data[i][j+1]+data[i+1][j+1]+data[i+1][j]);
						 if (centerValue < isovalues[m])
						 {
							 squareIndexs[m]=10;
						 }
					 }
					 else if(squareIndexs[m] == 10)//0x10
					 {
						 float centerValue =(1/4.0) *(data[i][j]+data[i][j+1]+data[i+1][j+1]+data[i+1][j]);
						 if (centerValue < isovalues[m])
						 {
							 squareIndexs[m]=5;
						 }
					 }
					 int edgeIndex1=0,edgeIndex2=0;
					 for(int k=0;SegmentTable[squareIndexs[m]][k] !=-1;k=k+2)
					 {
						 edgeIndex1 = SegmentTable[squareIndexs[m]][k];
						 edgeIndex2 = SegmentTable[squareIndexs[m]][k+1];
						 addPointToLine(edgeIndex1,edgeIndex2,i,j,data,isovalues[m],pathLinesV[m],startLongitude,longitudeGridSpace, startLatitude,latitudeGridSpace);
					 }	
				 }
			 }
		 }
	 }
	 for (int i=0;i<pathLinesV.size();++i)
	 {
		 for (int j=0;j<pathLinesV[i].size();++j)
		 {
			 if (abs(pathLinesV[i][j].startPoint.x -pathLinesV[i][j].endPoint.x)<=0.5 && abs(pathLinesV[i][j].startPoint.y -pathLinesV[i][j].endPoint.y)<=0.5  )
			 {
				// pathLinesV[i][j].points.pop_back();
				 //近似首尾相连
				 pathLinesV[i][j].isCircle = true;
				 pathLinesV[i][j].endPoint = pathLinesV[i][j].startPoint;

			 }
		 }
	 }


 }
}