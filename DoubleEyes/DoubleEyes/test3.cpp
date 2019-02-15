#include "stdafx.h"
#include <iostream>
#include <opencv2\core\core.hpp>
#include <opencv2\opencv.hpp> //resize方法位于此库中
#include <math.h>
#include <fstream> // 文件读写操作
#include "MatlabNull.h" 

using namespace std;
using namespace cv;

#define BASELINE 6.0
#define FOCUS 80.0
#define DX 0.4
#define DY DX
#define SRCSIZE 400
#define FUSESIZE 200
//#define U0 int(SRCSIZE/2)
//#define V0 U0
#define ONEPLEX 10.0/15.0

// 关键点像素坐标转相机坐标
vector<vector<Point3f>> keyPointsFromPlex2Camera(vector<Point2i> leftImg, vector<Point2i> rightImg, Size imgSize) {
	vector<vector<Point3f>> cameraCoors;
	vector<Point3f> leftCameraCoor, rightCameraCoor;
	int U0 = imgSize.width / 2;
	int V0 = imgSize.height / 2;
	for (int i = 0; i < leftImg.size(); i++) {
		float delta = DX*(leftImg[i].y - rightImg[i].y);
		leftCameraCoor.push_back(Point3f(DX*BASELINE*(leftImg[i].y - U0) / delta, DY*BASELINE*(leftImg[i].x - V0) / delta, BASELINE*FOCUS / delta));
		rightCameraCoor.push_back(Point3f(DX*BASELINE*(rightImg[i].y - U0) / delta, DY*BASELINE*(rightImg[i].x - V0) / delta, BASELINE*FOCUS / delta));
	}
	cameraCoors.push_back(leftCameraCoor);
	cameraCoors.push_back(rightCameraCoor);
	return cameraCoors;
}

// 求平面方程的四个系数
MatrixXf getScanPanelFactors(Mat keyPoints) {
	return getFactor(keyPoints);
}

// 计算平面上的各点坐标，并将其转换为像素坐标
vector<Mat> getEachCoorFromCamera2Plex(MatrixXf factors, Size imgSize) {
	vector<Mat> plexcoor;
	Mat ucoor = Mat::zeros(imgSize, CV_32SC1);
	Mat vcoor = Mat::zeros(imgSize, CV_32SC1);
	int U0 = imgSize.width / 2;
	int V0 = imgSize.height / 2;
	for (int i = 0; i < imgSize.width; i++) {
		for (int j = 0; j < imgSize.height; j++) {
			float x = DX*i;
			float y = DY*j;
			float z = -(factors(0)*x + factors(1)*y + factors(3)) / factors(2);
			int u = ((FOCUS*x) / (DX*z));
			int v = ((FOCUS*y) / (DY*z));
			ucoor.at<int>(j, i) = u;
			vcoor.at<int>(j, i) = v;
		}
	}
	plexcoor.push_back(ucoor);//col
	plexcoor.push_back(vcoor);//row
	return plexcoor;
}
vector<Mat> getEachCoorFromCamera2Plex2(MatrixXf factors, Size imgSize) {
	ofstream fout("mattxt.txt");
	vector<Mat> plexcoor;
	Mat ucoor = Mat::zeros(imgSize, CV_32SC1);
	Mat vcoor = Mat::zeros(imgSize, CV_32SC1);
	for (int i = 0; i < imgSize.width; i++) {
		for (int j = 0; j < imgSize.height; j++) {
			float x = ONEPLEX*i;
			float y = ONEPLEX*j;
			float z = -(factors(0)*DX*i+ factors(1)*DY*j + factors(3)) / factors(2);
			fout << z << ",";
			float delta_x = BASELINE*FOCUS / z;
			int u = ((delta_x*x) / (DX*BASELINE));
			int v = ((delta_x*y) / (DY*BASELINE));
			ucoor.at<int>(j, i) = abs(u);
			vcoor.at<int>(j, i) = abs(v);
		}
		fout << ";" << endl;
	}
	fout.close();
	plexcoor.push_back(ucoor);//col
	plexcoor.push_back(vcoor);//row
	return plexcoor;
}

// 图像叠加
Mat getUltrasoundFuseImage(Mat backgroud, Mat ultrasound, vector<Mat> coorMat, Point2i start, float alpha) {
	Mat ucoor = coorMat[0];
	Mat vcoor = coorMat[1];
	for (int i = 0; i < ucoor.rows; i++) {
		for (int j = 0; j < ucoor.cols; j++) {
			int u = start.x + ucoor.at<int>(i, j);
			int v = start.y + vcoor.at<int>(i, j);
			backgroud.at<Vec3b>(v, u) = alpha*backgroud.at<Vec3b>(v, u) + (1 - alpha)*ultrasound.at<Vec3b>(i, j);
		}
	}
	return backgroud;
}

/*int main() {
	// 图像素材
	Mat background = Mat(Size(400, 400), CV_8UC3, Scalar::all(255));
	//Mat ultrasoundImg = Mat(Size(4, 4), CV_8UC3, Scalar::all(40));
	Mat ultrasoundImg = imread("D:/document/ffmpeg/cs.png");
	Size ultrasoundImgResize = Size(50, 50);
	resize(ultrasoundImg, ultrasoundImg, ultrasoundImgResize);
	// 关键坐标点
	vector<Point2i> leftPs, rightPs;
	leftPs.push_back(Point2i(60, 60));
	rightPs.push_back(Point2i(60, 69));
	leftPs.push_back(Point2i(60, 100));
	rightPs.push_back(Point2i(60, 109)); 
	leftPs.push_back(Point2i(50, 60));
	rightPs.push_back(Point2i(50, 69));
	//vector<vector<Point3f>> key3d = keyPointsFromPlex2Camera(leftPs, rightPs, Size(400,400));
	//cout << "three key points in left image and right image, get camera points:" << endl << key3d[0] << endl << key3d[1] << endl;
	float kpa[3][3] = { 150,150,500, 200,150,500, 200,200,450 };
	Mat keyPoints = Mat(3, 3, CV_32FC1, kpa);
	MatrixXf factorsL = getScanPanelFactors(keyPoints);
	//MatrixXf factorsL = getScanPanelFactors(Mat(key3d[0]).reshape(1, 3).clone());
	cout << "left coordinate get panel factor:" << endl << factorsL << endl;
	//MatrixXf factorsR = getScanPanelFactors(Mat(key3d[1]).reshape(1, 3).clone());
	//cout << "right coordinate get panel factor:" << endl << factorsR << endl;
	vector<Mat> plexcoors = getEachCoorFromCamera2Plex2(factorsL, ultrasoundImgResize);
	//cout << "get plex coordinate of each point on panel:" << endl << plexcoors[0] << endl << plexcoors[1] << endl;
	double maxx, maxy, minx, miny;
	minMaxLoc(plexcoors[0], &minx, &maxx);
	minMaxLoc(plexcoors[1], &miny, &maxy);
	cout << "max x:" << maxx << "  ;max y:" << maxy << endl;
	int maxxxx = maxx > maxy ? maxx : maxy;
	if (maxxxx>200) resize(background, background, Size(maxxxx * 2, maxxxx * 2));
	Mat remat = getUltrasoundFuseImage(background, ultrasoundImg, plexcoors, leftPs[0], 0.2);
	imshow("remat", remat);
	waitKey();
	system("pause");
	return 0;
}
*/