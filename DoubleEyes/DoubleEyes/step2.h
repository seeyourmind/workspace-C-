#pragma once
#include "stdafx.h"
#include <fstream> // 文件读写操作
#include <thread>
#include "MatlabNull.h" 

using namespace std;
using namespace cv;

#define BASELINE 6.0		//基线
#define FOCUS 80.0			//焦距
#define SRCSIZE 400			//背景图resize
#define FUSESIZE 200		//融合图resize
//#define U0 int(SRCSIZE/2)
//#define V0 U0
#define ONEPLEX 10.0/15.0	//图像坐标系中单位像素
#define DELTA 20			//
#define FUSIZE 200			//融合矩阵缩放指标

void step2processing(Mat background, Mat cs, Mat cs_sector, vector<Point2f> plexL, vector<Point2f> plexR);
Mat step2processing(Mat background, Mat cs, Mat cs_sector, vector<Point2f> plexL, vector<Point2f> plexR, double angle, Point2f startp, double alpha, double flag);