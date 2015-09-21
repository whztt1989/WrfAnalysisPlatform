#pragma once
#include<iostream>
#include<ctime>
#include<vector>
#include "isotools.h"

using namespace std;
/************************************************************************/
/* namespace:   MarchingSquares     
 * Description: �ṩʹ��MarchingSquares�������ɸ���ֵ������ֵ�ķ����ӿ�
 * Author: Wenshan Zhou
 * Date�� 2012-8-24
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
 // note the sub array is length 5, with a -1 at the end to mark end of array �������EdgeTable[]ֵһһ��Ӧ
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


const float V_EPSILON = 1.0f / 256.0f;//�����ֵ

/************************************************************************/
/* Funciton:   VertexInterp     
 * Description: ��ֵ����
 * Input:
	isovalue: ��ֵ��ֵ
	x1�� ��һ����������� x �±�
	y1�� ��һ����������� y �±�
	v1:  ��һ������ֵ
	x2�� �ڶ������������ x �±�
	y2�� �ڶ������������ y �±�
	v2:  �ڶ�������ֵ
 * Output: ������ֵ������꼰��ֵ��ֵ��Point2D����
 * Author: Wenshan Zhou
 * Date�� 2012-8-24
 * Modified : 2012-9-9 by Wenshan Zhou
************************************************************************/
static isotools::Point2D  VertexInterp( float isovalue, int x1,int y1,float v1, int x2,int y2,float v2,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
{
        float mu;
        isotools::Point2D p;

        if ( std::abs(v1-v2) < V_EPSILON )
		{
			p.x = y1 * longitudeGridSpace + startLongitude;//�������±껻��OpenGL����ϵ
			p.y = x1 * latitudeGridSpace + startLatitude; //�������±껻��OpenGL����ϵ
            return p;
		}

        mu = (isovalue - v1) / (v2 - v1);
		p.x = (y1 + mu * (float)(y2 - y1)) * longitudeGridSpace + startLongitude;//�������±껻��OpenGL����ϵ
		p.y = (x1 + mu * (float)(x2 - x1)) * latitudeGridSpace + startLatitude;//�������±껻��OpenGL����ϵ
		
        return p;
 }
 
 /************************************************************************/
/* Funciton:   getMiddlePoint     
 * Description: ��ý������ڱߵ��е㣨�������൱�ڸñߵ�ȫ�ֱ�ţ�
 * Input:
	edgeIndex: �������ڱ���cell�еľֲ���ţ�0,1,2,3��
	i�� ��������cell�����Ͻǵ������xֵ�±�
	j����������cell�����Ͻǵ������yֵ�±�
 * Output: isotools::Point2D  ���ظñߵ��е�
 * Author: Wenshan Zhou
 * Date�� 2012-9-9
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
 * Description: ͨ����ֵ��ý��������
 * Input:
	edgeIndex: �������ڱ���cell�еľֲ���ţ�0,1,2,3��
	i�� ��������cell�����Ͻǵ������xֵ�±�
	j����������cell�����Ͻǵ������yֵ�±�
	data����������ֵ��ά����
	isovalue����ֵ
 * Output: isotools::Point2D  ���ظý��������
 * Author: Wenshan Zhou
 * Date�� 2012-9-10
/************************************************************************/
 static isotools::Point2D getCutPoint(int edgeIndex,int i,int j,vector<vector<float> > &data,float isovalue,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 isotools::Point2D p;
	 switch(edgeIndex)
	 {
	 case 0:
		 return VertexInterp(isovalue,i+1,j,data[i+1][j],i,j,data[i][j],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace); //��node ����й�
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
  * Description: �����߶κϲ������������߶κϲ����򷵻�true�����򷵻�flase
  * type 0:��ʾ��ͷ��� ,1��ʾβ�ڵ�
 */
 static bool isMergeIsoLine( isotools::Point2D mid,int type,vector<isotools::Isoline> &pathLines,int m )
 {
	 for (int i=0;i<pathLines.size();++i)
	 {
		 if(i==m)continue;

		 if (mid == pathLines[i].startPoint && type ==0)//�����ߵ� ͷ��� �غϣ��򽫶̵��߶μӵ������߶Σ���ɾ���̵��߶�
		 {
			 if(pathLines[m].points.size() > pathLines[i].points.size())//�߶�m�ϳ�����i�еĵ��ƶ���m��
			 {
				 list<isotools::Point2D>::iterator pIt = pathLines[i].points.begin();//ָ��ͷ���
				 pIt++;//����ͷ���
				 isotools::Point2D point ;
				 while (pIt!=pathLines[i].points.end())
				 {
					 point= *pIt;//ȡֵ
					 pathLines[m].points.push_front(point);//��ͷ�����β���
					 pIt++;
				 }
				  //�޸�m��ͷ���
				  pathLines[m].startPoint = pathLines[i].endPoint;

				 //ɾ��i
				  vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
				  int k=0;
				  while (k!=i)//�ƶ����������߶�pathLines[i]��
				  {
					  lineIt++;
					  k++;
				  }
				  pathLines.erase(lineIt);//ɾ���߶�pathLines[i]
				 

			 }
			 else //�߶�i�ϳ�����m�еĵ��ƶ���i��
			 {
				 list<isotools::Point2D>::iterator pIt = pathLines[m].points.begin();//ָ��ͷ���
				 pIt++;//����ͷ���
				 isotools::Point2D point ;
				 while (pIt!=pathLines[m].points.end())
				 {
					 point= *pIt;//ȡֵ
					 pathLines[i].points.push_front(point);//��ͷ�����β���
					 pIt++;
				 }
				 //�޸�i��ͷ���
				 pathLines[i].startPoint = pathLines[m].endPoint;
				
				 //ɾ��m
				 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
				 int k=0;
				 while (k!=m)//�ƶ����������߶�pathLines[m]��
				 {
					  k++;
					 lineIt++;
				 }
				 pathLines.erase(lineIt);//ɾ���߶�pathLines[m]
			 }
			 return true;
		 }
		 else if (mid == pathLines[i].endPoint && type ==1)//�����ߵ� β��� �غϣ��򽫶̵��߶μӵ������߶Σ���ɾ���̵��߶�
		 {
			 if(pathLines[m].points.size() > pathLines[i].points.size())//�߶�m�ϳ�����i�еĵ��ƶ���m��
			 {
				 list<isotools::Point2D>::iterator pIt = pathLines[i].points.end();//ָ��β���
				 pIt--;//����end
				 pIt--;//����β���
				 isotools::Point2D point ;
				 while (pIt!=pathLines[i].points.begin())
				 {
					 point= *pIt;//ȡֵ
					 pathLines[m].points.push_back(point);//��β�����β���
					 pIt--;
				 }
				 //��pushͷ���
				 point= *pIt;//ȡֵ
				 pathLines[m].points.push_back(point);//��β������
				 //�޸�m��β���
				 pathLines[m].endPoint = pathLines[i].startPoint;

				 //ɾ��i
				 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
				 int k=0;
				 while (k!=i)//�ƶ����������߶�pathLines[i]��
				 {
					  k++;
					 lineIt++;
				 }
				 pathLines.erase(lineIt);//ɾ���߶�pathLines[i]

			 }
			 else //�߶�i�ϳ�����m�еĵ��ƶ���i��
			 {
				 list<isotools::Point2D>::iterator pIt = pathLines[m].points.end();//ָ��β���
				 pIt--;//����end
				 pIt--;//����β���
				 isotools::Point2D point ;
				 while (pIt!=pathLines[m].points.begin())
				 {
					 point= *pIt;//ȡֵ
					 pathLines[i].points.push_back(point);//��β�����β���
					 pIt--;
				 }
				 //��pushͷ���
				 point= *pIt;//ȡֵ
				 pathLines[i].points.push_back(point);//��β������
				 //�޸�i��β���
				 pathLines[i].endPoint = pathLines[m].startPoint;
				
				 //ɾ��m
				 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
				 int k=0;
				 while (k!=m)//�ƶ����������߶�pathLines[m]��
				 {
					  k++;
					 lineIt++;
				 }
				 pathLines.erase(lineIt);//ɾ���߶�pathLines[m]
			 }
			  return true;
		 }
		 else if (mid == pathLines[i].endPoint && type ==0)//m�ߵ�ͷ�����i��β��� �غϣ���m�ӵ�i��
		 {
			 //��m�еĵ��ƶ���i��
			 list<isotools::Point2D>::iterator pIt = pathLines[m].points.begin();//ָ��ͷ���
			 pIt++;//����ͷ���
			 isotools::Point2D point ;
			 while (pIt!=pathLines[m].points.end())
			 {
				 point= *pIt;//ȡֵ
				 pathLines[i].points.push_back(point);//��β�����β���
				 pIt++;
			 }
			 //�޸�i��β���
			 pathLines[i].endPoint = pathLines[m].endPoint;

			 //ɾ��m
			 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
			 int k=0;
			 while (k!=m)//�ƶ����������߶�pathLines[m]��
			 {
				  k++;
				 lineIt++;
			 }
			 pathLines.erase(lineIt);//ɾ���߶�pathLines[m]
			 return true;
		 }
		 else if (mid == pathLines[i].startPoint && type ==1)//m�ߵ�β�����i��ͷ��� �غϣ���i�ӵ�m��
		 {
			 //��m�еĵ��ƶ���i��
			 list<isotools::Point2D>::iterator pIt = pathLines[i].points.begin();//ָ��ͷ���
			 pIt++;//����ͷ���
			 isotools::Point2D point ;
			 while (pIt!=pathLines[i].points.end())
			 {
				 point= *pIt;//ȡֵ
				 pathLines[m].points.push_back(point);//��β�����β���
				 pIt++;
			 }
			 //�޸�m��ͷ���
			 pathLines[m].endPoint = pathLines[i].endPoint;

			 //ɾ��i
			 vector<isotools::Isoline>::iterator lineIt = pathLines.begin();
			 int k=0;
			 while (k!=i)//�ƶ����������߶�pathLines[i]��
			 {
				  k++;
				 lineIt++;
			 }
			 pathLines.erase(lineIt);//ɾ���߶�pathLines[i]
			 return true;
		 }
	 }
	 return false;
 }

 /************************************************************************/
/* Funciton:   addPointToLine     
 * Description:  �жϵ�ǰ�����ϵĵ��Ƿ������е�ֵ�ߵ���β���غϣ����ǣ�����Դ��غϵ㣬�ٽ���һ������뵽�׻�β���жϼ�����Ƿ������������߶ζ˵��غϣ��Խ��кϲ���������������һ����ֵ��
 * Input:
	edgeIndex1: ����1���ڱ���cell�еľֲ���ţ�0,1,2,3��
	edgeIndex2: ����2���ڱ���cell�еľֲ���ţ�0,1,2,3�� �����������㹹����cell�е�һ����ֵ�ߣ�
	i�� ��������cell�����Ͻǵ������xֵ�±�
	j����������cell�����Ͻǵ������yֵ�±�
	data����������ֵ��ά����
	isovalue����ֵ
	pathLines�����ظõ�ֵ��Ӧ�ĸ�����ֵ������ ��ÿ��������һ��Isoline��
 * Output: isotools::Point2D  ���ظñߵ��е�
 * Author: Wenshan Zhou
 * Date�� 2012-9-9
/************************************************************************/
 static void  addPointToLine(int edgeIndex1,int edgeIndex2 , int i,int j,vector<vector<float> > &data,float isovalue,vector<isotools::Isoline> &pathLines,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 //�жϵ�ǰ���ϵĵ��Ƿ������е�ֵ�ߵ���β���غϣ����ǣ�����Դ��غϵ㣬�ٽ���һ������뵽�׻�β����������һ����ֵ��
	
	  isotools::Point2D mid1 = getMiddlePoint(edgeIndex1,i,j);//�����������
	  isotools::Point2D mid2 = getMiddlePoint(edgeIndex2,i,j);//�����������

	  int m;
	  for (m= 0;m<pathLines.size();++m)
	  {
		  if (pathLines[m].isCircle)
		  {
			  continue;//���ڹ��ɻ��Ĳ����ж�
		  }
		  //�����ϵĵ�����β��㶼��ͬ
		  if ((mid1 == pathLines[m].startPoint && mid2 == pathLines[m].endPoint)||(mid2 == pathLines[m].startPoint && mid1 == pathLines[m].endPoint))
		  {
			  //isotools::Point2D point = pathLines[m].points.front();//���ص�һ��Ԫ��  �����ٶ��һ��Ԫ��,����ֻ��isCircle���
			  //pathLines[m].points.push_back(point);//������һ��Ԫ����ĩβ���γɻ�·  �����1,2,3���ɻ�·����ֻ�����1,2,3�����ٱ���1,2,3,1��
			  pathLines[m].isCircle = true;//�γɻ�·
			  break;
		  } 
		  else if (mid1 == pathLines[m].startPoint )//��m1��ͷ����غ�
		  {//����ͷ���ǰ������һ���㣬������ͷ���
			  isotools::Point2D point = getCutPoint(edgeIndex2,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			  pathLines[m].points.push_front(point);
			  pathLines[m].startPoint = mid2;
			  isMergeIsoLine(mid2,0,pathLines,m);//���кϲ�����
			  break;
		  }
		  else if (mid1 == pathLines[m].endPoint)//��m1��β����غ�
		  {
			  isotools::Point2D point = getCutPoint(edgeIndex2,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			  pathLines[m].points.push_back(point);
			  pathLines[m].endPoint = mid2;
			  isMergeIsoLine(mid2,1,pathLines,m);//���кϲ�����
			  break;
		  }
		  else if (mid2 == pathLines[m].startPoint)//��m2��ͷ����غ�
		  { //����ͷ���ǰ������һ���㣬������ͷ���
			  isotools::Point2D point = getCutPoint(edgeIndex1,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			  pathLines[m].points.push_front(point);
			  pathLines[m].startPoint = mid1;
			  isMergeIsoLine(mid1,0,pathLines,m);//���кϲ�����
			  break;
		  }
		  else if (mid2 == pathLines[m].endPoint)//��m2��β����غ�
		  {
			  isotools::Point2D point = getCutPoint(edgeIndex1,i,j,data,isovalue,startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			  pathLines[m].points.push_back(point);
			  pathLines[m].endPoint = mid1;
			  isMergeIsoLine(mid1,1,pathLines,m);//���кϲ�����
			  break;
		  }
	  }
	  if(m == pathLines.size())//˵��������û�����κ�һ����ֵ����ӣ����´���һ����ֵ��
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
		  pathLines.push_back(isoList);//����һ����ֵ��
	  }
 }

 /************************************************************************/
/* Funciton:   doMarchingSquares     ���˷��������� GPU ���м��١�
 * Description: Marching Squares �㷨��ʵ��
 * Input:
	data: ��������ֵ��ά����
	isovalue�� ��ֵ��ֵ
	pathPoints�� ���ɵĵ�ֵ������
 * Output: void
 * Author: Wenshan Zhou
 * Date�� 2012-8-23
 * Modified: 2012-9-9 by Wenshan Zhou  
/************************************************************************/
 static void   doMarchingSquares(vector<vector<float> > &data,float isovalue,vector<isotools::Point2D> &pathPoints,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 pathPoints.clear();
	 for(int i =0; i<data.size()-1;++i)//����ɨ
	 {
		 for(int j=0;j<data[i].size()-1;++j)//����ɨ
		 {
			 int squareIndex = 0;

			 if(data[i][j] > isovalue) squareIndex|=8;
			 if(data[i][j+1] > isovalue) squareIndex|=4;
			 if(data[i+1][j+1] > isovalue) squareIndex|=2;
			 if(data[i+1][j] > isovalue) squareIndex|=1;


			 if(EdgeTable[squareIndex]==0 )
				 continue;

			 //����������
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

			isotools::Point2D vertlist[4];//���ÿ���ϸ�һ�����㣬���ĸ�

			 if (EdgeTable[squareIndex] & 8)//0�ű����е� 
			 {
				 vertlist[0] = VertexInterp(isovalue,i+1,j,data[i+1][j],i,j,data[i][j],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace); //��node ����й�
			 }

			 if (EdgeTable[squareIndex] & 4)//1�ű����е�
			 {
				 vertlist[1] = VertexInterp(isovalue,i,j,data[i][j],i,j+1,data[i][j+1],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			 }

			 if (EdgeTable[squareIndex] & 2)//2�ű����е�
			 {
				 vertlist[2] = VertexInterp(isovalue,i,j+1,data[i][j+1],i+1,j+1,data[i+1][j+1],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			 }

			 if (EdgeTable[squareIndex] & 1)//3�ű����е�
			 {
				 vertlist[3] = VertexInterp(isovalue,i+1,j+1,data[i+1][j+1],i+1,j,data[i+1][j],startLongitude,longitudeGridSpace,startLatitude,latitudeGridSpace);
			 }
			 //collect the pathPoints,�������һ��
			 for(int k=0;SegmentTable[squareIndex][k] !=-1;k=k+2)
			 {
				 pathPoints.push_back(vertlist[SegmentTable[squareIndex][k]]);
				 pathPoints.push_back(vertlist[SegmentTable[squareIndex][k+1]]);
			 }


		 }
	 }
 }
 /************************************************************************/
/* Funciton:   doMarchingSquares      ���˷��������� CPU��
 * Description: Marching Squares �㷨��ʵ��
 * Input:
	data: ��������ֵ��ά����
	isovalue�� ��ֵ��ֵ
	pathLines�� ���ظõ�ֵ�µĸ�����ֵ������
 * Output: void
 * Author: Wenshan Zhou
 * Date�� 2012-9-10
/************************************************************************/
 static void   doMarchingSquares(vector<vector<float> > &data,float isovalue,vector<isotools::Isoline> &pathLines,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 pathLines.clear();
	 for(int i =0; i<data.size()-1;++i)//����ɨ
	 {
		 for(int j=0;j<data[i].size()-1;++j)//����ɨ
		 {
			 int squareIndex = 0;

			 if(data[i][j] > isovalue) squareIndex|=8;
			 if(data[i][j+1] > isovalue) squareIndex|=4;
			 if(data[i+1][j+1] > isovalue) squareIndex|=2;
			 if(data[i+1][j] > isovalue) squareIndex|=1;


			 if(EdgeTable[squareIndex]==0 )
				 continue;

			 //����������
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

			 //collect the pathPoints,�������һ��
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
/* Funciton:   doMarchingSquares      ���˷��������� CPU�� 
 * Description: Marching Squares �㷨��ʵ��
 * Input:
	data: ��������ֵ��ά����
	isovalues�� ��ֵ��ֵ����
	pathLinesV�� ���ظõ�ֵ�����µ����и�����ֵ������ļ���
 * Output: void
 * Author: Wenshan Zhou
 * Date�� 2012-9-10
/************************************************************************/
 static void   doMarchingSquares(vector<vector<float> > &data,vector<float> &isovalues,vector< vector<isotools::Isoline>> &pathLinesV,float startLongitude,float longitudeGridSpace,float startLatitude,float latitudeGridSpace)
 {
	 pathLinesV.clear();
	 vector<int> squareIndexs;
	 squareIndexs.resize(isovalues.size());
	 pathLinesV.resize(isovalues.size());

	 for(int i =0; i<data.size()-1;++i)//����ɨ
	 {
		 for(int j=0;j<data[i].size()-1;++j)//����ɨ
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
					 //����������
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
				 //������β����
				 pathLinesV[i][j].isCircle = true;
				 pathLinesV[i][j].endPoint = pathLinesV[i][j].startPoint;

			 }
		 }
	 }


 }
}