#pragma once
#include "stdafx.h"
#include <Eigen\Dense>

using namespace Eigen;

MatrixXf getFactor(Mat xs);

/**
* ����Eigen���������ʵ��Matlab�е�null������������η�����Ax=0
* Ŀǰֻ֧�ּ���ߴ�Ϊ3*4�Ĺؼ����������
* ʹ��SVD�ֽ⣬����Matlab��null��Դ����ʵ��
*/
