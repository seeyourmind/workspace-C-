#pragma once
#include "stdafx.h"
#include <core\core.hpp>
#include <opencv2\opencv.hpp> //resize����λ�ڴ˿���
#include <highgui\highgui.hpp>
#include <iostream>
#include <math.h> // ���Ǻ�����Ҫ
#include <fstream> // д�ļ�
#include "MatlabNull.h" // ��ȡĿ��ƽ��

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
* ����ͼ���ں�����˼·��
* ǰ�᣺̽ͷ��ǣ�����һͷ������һͷԲ��
* ���裺	1��ͼ��������ȷ����ǵ㣬ʶ���̽ͷλ�ã�
		2������̽ͷ���ֵ��Ӳ�����ؼ�����Ӳ������һ�������һ�����ģ�Բ�ε�Բ�ģ���
		3�������Ӳ������ά����
		4�����ݹؼ�����ά����ȷ��ƽ�淽��
		5��ȷ��ƽ��֮�󽫳���ͼ��Ƕ��ƽ��
		6������ͼ��������Ҫ����ƽ��Ŀռ�λ�����ʵ�ת��
		7������ƽ���ڳ���ͼ������Ƶͼ����Ӧ�ó��ֵ�λ��
*/