#include "stdafx.h"
#include "MatlabNull.h"

// 浮点相对精度，等同于MATLAB中的eps(x)
double eps(double x) { double xp = std::abs(x); double x1 = std::nextafter(xp, xp + 1.0f); return x1 - xp; }
// 浮点相对精度，等同于MATLAB中的eps(single(x))
double eps(float x) { float xp = std::abs(x); double x1 = std::nextafter(xp, xp + 1.0f); return x1 - xp; }
// 小数保留小数点后四位
double fourfloor(double a) { double b = floor(a * 10000.000f + 0.5) / 10000.000f; return b; }

// 计算Ax=b，得到系数矩阵
MatrixXf getFactor(Mat xs) {
	MatrixXf m(3,4);
	if (xs.rows > 3 || xs.cols > 3) {
		cout << "your mat size is too big" << endl << xs << endl;
		system("pause");
		exit(-1);
	}
	for (int i = 0; i < xs.rows; i++) {
		for (int j = 0; j < xs.cols; j++) {
			m(i, j) = xs.at<float>(i, j);
		}
		m(i, 3) = 1;
	}
	//cout << "Here is the matrix m:" << endl << m << endl;
	JacobiSVD<MatrixXf> svd(m, ComputeFullV);
	MatrixXf s = svd.singularValues();
	MatrixXf V = svd.matrixV();
	double tol = max(3, 4) * eps(double(s.maxCoeff()));
	int r = (s.array() > tol).select(MatrixXf::Ones(s.rows(),s.cols()),MatrixXf::Zero(s.rows(),s.cols())).sum();
	MatrixXf factorM = V.rightCols(m.cols() - r);
	for (int i = 0; i < factorM.size(); i++) {
		double f = fourfloor(factorM(3)) > 0 ? factorM(3) : 1;
		factorM(i) = fourfloor(factorM(i) / f);
		
	}
	// cout << "factor:" << endl << factorM << endl;
	return factorM;
}

// 测试函数
void test() {
	float kpa[3][4] = { 1,2,3,1, 0,1,1,1, 1,0,1,1 };
	Mat keyPoints = Mat(3, 4, CV_32FC1, kpa);
	MatrixXf factor = getFactor(keyPoints);
	cout << "factor:" << endl<<factor << endl;
	system("pause");
}

