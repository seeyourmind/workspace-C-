#include "stdafx.h"
#include "MatlabNull.h"

// ������Ծ��ȣ���ͬ��MATLAB�е�eps(x)
double eps(double x) { double xp = std::abs(x); double x1 = std::nextafter(xp, xp + 1.0f); return x1 - xp; }
// ������Ծ��ȣ���ͬ��MATLAB�е�eps(single(x))
double eps(float x) { float xp = std::abs(x); double x1 = std::nextafter(xp, xp + 1.0f); return x1 - xp; }
// С������С�������λ
double fourfloor(double a) { double b = floor(a * 10000.000f + 0.5) / 10000.000f; return b; }

// ����Ax=b���õ�ϵ������
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

// ���Ժ���
void test() {
	float kpa[3][4] = { 1,2,3,1, 0,1,1,1, 1,0,1,1 };
	Mat keyPoints = Mat(3, 4, CV_32FC1, kpa);
	MatrixXf factor = getFactor(keyPoints);
	cout << "factor:" << endl<<factor << endl;
	system("pause");
}

