#pragma once
#include "stdafx.h"
#include <Eigen\Dense>

using namespace Eigen;

MatrixXf getFactor(Mat xs);

/**
* 借助Eigen矩阵运算库实现Matlab中的null函数，计算齐次方程组Ax=0
* 目前只支持计算尺寸为3*4的关键点坐标矩阵
* 使用SVD分解，按照Matlab中null的源代码实现
*/
