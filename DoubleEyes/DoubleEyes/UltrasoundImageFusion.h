#pragma once
#include "stdafx.h"
#include <iostream>
#include <opencv2\core\core.hpp>
#include <opencv2\opencv.hpp> //resize方法位于此库中
#include <math.h>
#include <fstream> // 文件读写操作
#include "MatlabNull.h" 

using namespace std;
using namespace cv;

#define BASELINE 6.0
#define FOCUS 80.0
#define DX 0.4
#define DY DX
#define SRCSIZE 400
#define FUSESIZE 200
//#define U0 int(SRCSIZE/2)
//#define V0 U0
#define ONEPLEX 10.0/15.0

class UltrasoundImageFusion {
public:
	static void ultrasoundImageFusion(Mat background, Mat ultrasoundImg, vector<Point2f> leftPs, vector<Point2f> rightPs, Point2f startPoint, double alpha);
private:
	vector<vector<Point3f>> keyPointsFromPlex2Camera(vector<Point2f> leftImg, vector<Point2f> rightImg, Size imgSize);
	MatrixXf getScanPanelFactors(Mat keyPoints);
	vector<Mat> getEachCoorFromCamera2Plex(MatrixXf factors, Size imgSize);
	Mat getUltrasoundFuseImage(Mat backgroud, Mat ultrasound, vector<Mat> coorMat, Point2f start, float alpha);
};