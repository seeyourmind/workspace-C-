#pragma once
#include "stdafx.h"
#include <core\core.hpp>
#include <opencv2\opencv.hpp> //resize方法位于此库中
#include <highgui\highgui.hpp>
#include <iostream>
#include <math.h> // 三角函数需要
#include <fstream> // 写文件

#define PI 3.1415926


Mat mergeFrame(Mat frame_up, Mat img, int bias, double percent);
vector<Mat> divideFrame(Mat &frame, bool vertical = 0);
Mat combineFrame(Mat up, Mat down, bool vertical = 0);
int doubleEyesVideoFuse(String srcVideoPath, String outVideoPath, String imgPath, bool vertical = true);
Mat getDeepImage(Mat img1, Mat img2);
Mat getDeep(Size imgSise, int angle, double block_height=0.1);
Mat getDisparity(Mat deep, double focus = 1, double baseline = 1);
Mat mergeFrameByAngle(Mat frame, Mat img, int angle, double percent=0.5, int bias=10);

String srcVideoPath = "D:/document/ffmpeg";
String outVideoPath = "D:/document/ffmpeg/Cout";
String imgPath = "C:/Users/Fyzer/Desktop/GitHubUniverse.png";