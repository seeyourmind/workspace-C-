#pragma once
#include "stdafx.h"
#include <iostream>
#include <opencv2\core\core.hpp>
#include <Eigen\Dense>
#include <math.h>

using namespace std;
using namespace cv;
using namespace Eigen;

double eps(double x);
double eps(float x);
double fourfloor(double a);
MatrixXf getFactor(Mat xs);
void test();

/**
* ����Eigen���������ʵ��Matlab�е�null������������η�����Ax=0
* Ŀǰֻ֧�ּ���ߴ�Ϊ3*4�Ĺؼ����������
* ʹ��SVD�ֽ⣬����Matlab��null��Դ����ʵ��
*/
