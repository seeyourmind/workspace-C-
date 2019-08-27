#include "stdafx.h"
#include "step2.h"
#include <typeinfo>
#include <fstream>

double onePlexForCamera = 1.0;//相机坐标系中单位像素
float maxvalue_xl = 0.0;
float maxvalue_xr = 0.0;
float maxvalue_y = 0.0;
float minvalue_xl = 10000.0;
float minvalue_xr = 10000.0;
float minvalue_y = 10000.0;

// 关键点像素坐标转相机坐标
vector<vector<Point3f>> keyPointsFromPlex2Camera(vector<Point2f> leftImg, vector<Point2f> rightImg, Size imgSize) {
	vector<vector<Point3f>> cameraCoors;
	vector<Point3f> leftCameraCoor, rightCameraCoor;
	double U0 = 1.0*imgSize.width / 2.;
	double V0 = 1.0*imgSize.height / 2.;
	for (int i = 0; i < leftImg.size(); ++i) {
		double delta = ONEPLEX*(leftImg[i].x - rightImg[i].x);
		leftCameraCoor.push_back(Point3f(ONEPLEX*BASELINE*(leftImg[i].x - U0) / delta, ONEPLEX*BASELINE*(leftImg[i].y - V0) / delta, BASELINE*FOCUS / delta));
		rightCameraCoor.push_back(Point3f(ONEPLEX*BASELINE*(rightImg[i].x - U0) / delta, ONEPLEX*BASELINE*(rightImg[i].y - V0) / delta, BASELINE*FOCUS / delta));
	}
	onePlexForCamera = (leftCameraCoor[0].x - leftCameraCoor[1].x) / (leftImg[0].x - leftImg[1].x);
	cameraCoors.push_back(leftCameraCoor);
	cameraCoors.push_back(rightCameraCoor);
	return cameraCoors;
}

// 求平面方程的四个系数
MatrixXf getScanPanelFactors(Mat keyPoints) {
	return getFactor(keyPoints);
}

// 平面坐标
Mat matlabversion(MatrixXf factors, Mat background, Mat cs, Mat cs_sector, double angle, Point2f startp, double transparency) {
	// 传入参数本地化，提速
	double A = factors(0);
	double B = factors(1);
	double C = factors(2);
	double D = factors(3);
	int rows = cs.rows;
	int cols = cs.cols;

	// 像素坐标
	Mat plex_u_l = Mat::zeros(cs.size(), CV_32FC1);
	Mat plex_u_r = Mat::zeros(cs.size(), CV_32FC1);
	Mat plex_v = Mat::zeros(cs.size(), CV_32FC1);
	double tempti = 0.;
	double temptj = 0.;
	for (int i = 0; i < rows; ++i) {
		float *ulp = plex_u_l.ptr<float>(i);
		float *urp = plex_u_r.ptr<float>(i);
		float *vp = plex_v.ptr<float>(i);
		for (int j = 0; j < cols; ++j) {
			float x = onePlexForCamera*j;
			float y = onePlexForCamera*i;
			float z= -(A*x + B*y + D) / C;
			ulp[j] = FOCUS*x / z;
			urp[j] = FOCUS*(x - BASELINE) / z;
			vp[j] = FOCUS*y / z;
		}
	}

	// 标准化
	double max_ul, max_ur, max_v, min_ul, min_ur, min_v, ul_length, ur_length, v_length;
	Point maxp, minp;
	minMaxLoc(plex_u_l, &min_ul, &max_ul, &minp, &maxp);
	minMaxLoc(plex_u_r, &min_ur, &max_ur, &minp, &maxp);
	minMaxLoc(plex_v, &min_v, &max_v, &minp, &maxp);
	ul_length = max_ul - min_ul;
	ur_length = max_ur - min_ur;
	v_length = max_v - min_v;
	for (int i = 0; i < rows; ++i) {
		float *ulp = plex_u_l.ptr<float>(i);
		float *urp = plex_u_r.ptr<float>(i);
		float *vp = plex_v.ptr<float>(i);
		for (int j = 0; j < cols; ++j) {
			ulp[j] = plex_u_l.cols*ulp[j] / ul_length;
			urp[j] = plex_u_r.cols*urp[j] / ur_length;
			vp[j] = plex_u_l.rows*vp[j] / v_length;
		}
	}

	// 求融合坐标
	double alpha = 0.0;
	double alpha_ = PI*(90-angle) / 180;
	Mat x = Mat::zeros(cs.size(), CV_32FC1);
	Mat y = Mat::zeros(cs.size(), CV_32FC1);
	Mat z_;
	background.copyTo(z_);
	for (int i = 0; i < rows; ++i) {
		float *xp = x.ptr<float>(i);
		float *yp = y.ptr<float>(i);
		float *up = plex_u_l.ptr<float>(i);
		float *vp = plex_v.ptr<float>(i);
		for (int j = 0; j < cols; ++j) {
			if (j == 0) {
				alpha = PI * 0 / 180;
			}
			else if (i == 0) {
				alpha = PI * 90 / 180;
			}
			else {
				alpha = atan(up[j] / vp[j]);
			}
			double d = sqrt(pow(up[j], 2) + pow(vp[j], 2));
			xp[j] = d*cos(alpha - alpha_);
			yp[j] = d*sin(alpha - alpha_);
		}
	}

	// 标准化
	double max_x, max_y, min_x, min_y, x_length, y_length;
//	Point maxp, minp;
	minMaxLoc(x, &min_x, &max_x, &minp, &maxp);
	minMaxLoc(y, &min_y, &max_y, &minp, &maxp);
	x_length = max_x - min_x;
	y_length = max_y - min_y;
	for (int i = 0; i < rows; ++i) {
		float *xp = x.ptr<float>(i);
		float *yp = y.ptr<float>(i);
		for (int j = 0; j < cols; ++j) {
			xp[j] = cols*xp[j] / x_length;
			yp[j] = rows*yp[j] / y_length;
		}
	}

	// 叠加超声
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			//int x_ = floor(startp.y + x.ptr<float>(i)[j]) - 100;
			int x_ = floor(startp.y + x.ptr<float>(i)[j])-300;
			if (x_ < 0) x_ = 0;
			//int y_ = floor(startp.x + y.ptr<float>(i)[j]) - 70 + 10;
			int y_ = floor(startp.x + y.ptr<float>(i)[j])-990+10;
			if (y_ < 0) y_ = 0;
			if (x_ > background.rows - 1) x_ = background.rows - 1;
			if (y_ > background.cols - 1) y_ = background.cols - 1;
			// 根据遮罩判断
			if (cs_sector.ptr<uchar>(i)[j] == 255) {
				z_.ptr<Vec3b>(x_)[y_] = 0.9*background.ptr<Vec3b>(x_)[y_] + transparency*cs.ptr<Vec3b>(i)[j];
			}
		}
	}
	//imshow("z_", z_);
	//waitKey(0);
	return z_;
}

void step2processing(Mat background, Mat cs, Mat cs_sector, vector<Point2f> plexL, vector<Point2f> plexR) {
	vector<vector<Point3f>> camraCoor = keyPointsFromPlex2Camera(plexL, plexR, background.size());
	MatrixXf factors = getScanPanelFactors(Mat(camraCoor[0]).reshape(1, 3).clone());
	matlabversion(factors, background, cs, cs_sector, 60, Point2f(38, 132), 0.6);
}

Mat step2processing(Mat background, Mat cs, Mat cs_sector, vector<Point2f> plexL, vector<Point2f> plexR, double angle, Point2f startp, double alpha, double flag) {
	cout << startp << endl;
	vector<vector<Point3f>> camraCoor = keyPointsFromPlex2Camera(plexL, plexR, background.size());
	MatrixXf factors = getScanPanelFactors(Mat(camraCoor[0]).reshape(1, 3).clone());
	if (flag == 0 || flag == 1 || flag == 3) { angle = abs(angle); }
	Mat z = matlabversion(factors, background, cs, cs_sector, 90+abs(angle), startp, alpha);
	//circle(z, Point2f(startp.y, startp.x), 10, Scalar(0, 255, 255));
	return z;
}
 
int step2test() {
	// 准备图像数据
	cout << "准备图像数据" << endl;
	string root = "D:/UserCode/MatLab/BinocularUltrasoundImage/";
	Mat cs = imread(root + "siatcs_copy.png");
	Mat background = imread(root + "segmentation_5.jpg");
	resize(background, background, Size(floor(background.cols*0.1), floor(background.rows*0.1)));
	Mat cs_sector = imread(root + "siatcs_sector_copy.png");
	cvtColor(cs_sector, cs_sector, CV_RGB2GRAY);
	resize(cs, cs, Size(180, 180));
	resize(cs_sector, cs_sector, Size(180, 180));
	//imshow("background", background);
	//imshow("cs", cs);
	//imshow("mask", cs_sector);
	// 像素点转相机
	cout << "像素点转相机" << endl;
	vector<Point2f> plexL, plexR;
	plexL.push_back(Point2f(99.654816, 150.16751));
	plexL.push_back(Point2f(94.651764, 141.72487));
	plexL.push_back(Point2f(215.5076, 70.106598));
	plexR.push_back(Point2f(69.139099, 150.25458));
	plexR.push_back(Point2f(64.118835, 141.8511));
	plexR.push_back(Point2f(185.14618, 69.549042));
	/*
	vector<vector<Point3f>> camraCoor = keyPointsFromPlex2Camera(plexL, plexR, background.size());
	cout << camraCoor[0] << endl;
	cout << camraCoor[1] << endl;
	// 求平面方程的四个坐标系数
	cout << "求平面方程的四个坐标系数" << endl;
	MatrixXf factors = getScanPanelFactors(Mat(camraCoor[0]).reshape(1, 3).clone());
	cout << factors << endl;
	matlabversion(factors, background, cs, cs_sector, 60, Point2f(38, 132), 0.6);
	*/
	step2processing(background, cs, cs_sector, plexL, plexR);
 	system("pause");
	return 0;
}
