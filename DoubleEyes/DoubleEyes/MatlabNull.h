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
* 借助Eigen矩阵运算库实现Matlab中的null函数，计算齐次方程组Ax=0
* 目前只支持计算尺寸为3*4的关键点坐标矩阵
* 使用SVD分解，按照Matlab中null的源代码实现
*/
