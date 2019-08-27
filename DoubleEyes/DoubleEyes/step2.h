#pragma once
#include "stdafx.h"
#include <fstream> // �ļ���д����
#include <thread>
#include "MatlabNull.h" 

using namespace std;
using namespace cv;

#define BASELINE 6.0		//����
#define FOCUS 80.0			//����
#define SRCSIZE 400			//����ͼresize
#define FUSESIZE 200		//�ں�ͼresize
//#define U0 int(SRCSIZE/2)
//#define V0 U0
#define ONEPLEX 10.0/15.0	//ͼ������ϵ�е�λ����
#define DELTA 20			//
#define FUSIZE 200			//�ںϾ�������ָ��

void step2processing(Mat background, Mat cs, Mat cs_sector, vector<Point2f> plexL, vector<Point2f> plexR);
Mat step2processing(Mat background, Mat cs, Mat cs_sector, vector<Point2f> plexL, vector<Point2f> plexR, double angle, Point2f startp, double alpha, double flag);