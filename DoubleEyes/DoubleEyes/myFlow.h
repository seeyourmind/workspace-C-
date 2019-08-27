#pragma once
#include "stdafx.h"
#include <core\core.hpp>
#include <opencv2\opencv.hpp> //resize方法位于此库中
#include <highgui\highgui.hpp>
#include <iostream>
#include <math.h> // 三角函数需要
#include <fstream> // 写文件
#include "MatlabNull.h" // 获取目标平面

#define BASELINE 6
#define FOCUS 80
#define DX 0.4
#define DY DX
#define SRCSIZE 400
#define FUSESIZE 200
#define U0 int(SRCSIZE/2)
#define V0 U0
#define ONEPLEX 10.0/15.0


MatrixXf getTargetPanel(Mat keyPoints);
Mat getUltrasoundImageCoordinate(int row, int col, double A, double B, double C, double D, short Ox = 1, short Oy = 1);
Mat imageFuse(Mat img, Mat ultsoundImg, Mat coor, float alpha = 0.5);

/**
* 超声图像融合主体思路：
* 前提：探头标记，例如一头三角形一头圆形
* 步骤：	1、图像处理技术，确定标记点，识别出探头位置；
		2、计算探头部分的视差，三个关键点的视差（三角形一个顶点和一个中心，圆形的圆心）；
		3、根据视差计算三维坐标
		4、根据关键点三维坐标确定平面方程
		5、确定平面之后将超声图像嵌入平面
		6、超声图像坐标需要根据平面的空间位置做适当转换
		7、反算平面内超声图像在视频图像中应该出现的位置
*/