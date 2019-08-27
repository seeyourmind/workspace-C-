#pragma once
#include "stdafx.h"
#include <fstream> // �ļ���д����
#include <thread>
#include "MatlabNull.h" 


#define BASELINE 6.0		//����
#define FOCUS 80.0			//����
#define SRCSIZE 400			//����ͼresize
#define FUSESIZE 200		//�ں�ͼresize
//#define U0 int(SRCSIZE/2)
//#define V0 U0
#define ONEPLEX 10.0/15.0	//ͼ������ϵ�е�λ����
#define DELTA 20			//
#define FUSIONSIZE 150.0	//�ںϳߴ�

class UltrasoundImageFusion {
public:
	static void ultrasoundImageFusion(Mat backgroundL, Mat backgroundR, Mat ultrasoundImg, vector<Point2f> leftPs, vector<Point2f> rightPs, double angle, double alpha);
	static int maxvalue_xl;
	static int maxvalue_xr;
	static int maxvalue_y;
};