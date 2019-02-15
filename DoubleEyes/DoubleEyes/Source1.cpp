#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include <iostream>
#include <opencv2\imgproc\imgproc.hpp>//统计代码运行时间
#include "ProbeRecognition.h"
#include "UltrasoundImageFusion.h"

using namespace std;
using namespace cv;

Mat panduandianshifouzailunkounei(vector<Point2f> points, Mat src) {
	Mat dst = Mat(Size(src.cols, src.rows), CV_8UC3, Scalar::all(255));
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			// pointPolygonTest检测点是否在一个多边形内
			if (pointPolygonTest(points, Point2f(i, j), false) >= 0) {
				dst.at<Vec3b>(j, i) = src.at<Vec3b>(j, i);
			}
		}
	}/*
	imshow("pddsfzlkn", dst);
	waitKey(1000);*/
	return dst;
}

int main() {
	double t = (double)getTickCount();
	//Mat picl = imread("C:/Users/Fyzer/Desktop/probe_angle_left.png");
	//Mat picr = imread("C:/Users/Fyzer/Desktop/probe_angle_right.png");
	Mat picl = imread("D:/document/about programs/SIAT/论文/image/probe_left.png");
	Mat picr = imread("D:/document/about programs/SIAT/论文/image/probe_right.png");

	vector<Point2f> lktq_l = ProbeRecognition::probeRecognition(picl);
	vector<Point2f> lktq_r = ProbeRecognition::probeRecognition(picr);
	//cout << "get four corner points are:" << endl << lktq_l << endl << lktq_r << endl;
	
	Mat pddsfzlkn_picl = panduandianshifouzailunkounei(lktq_l, picl);
	Mat pddsfzlkn_picr = panduandianshifouzailunkounei(lktq_r, picr);
	
	imwrite("pddsfzlkn_l.jpg", pddsfzlkn_picl);
	imwrite("pddsfzlkn_r.jpg", pddsfzlkn_picr);
	
	Mat ultrasoundImg = imread("D:/document/ffmpeg/cs.png");
	lktq_l.pop_back();
	lktq_r.pop_back();
	UltrasoundImageFusion::ultrasoundImageFusion(picl, ultrasoundImg, lktq_l, lktq_r, lktq_l[0], 0.2);
	UltrasoundImageFusion::ultrasoundImageFusion(picr, ultrasoundImg, lktq_l, lktq_r, lktq_r[0], 0.2);//
	
	t -= (double)getTickCount();
	printf("execution time = %gms\n", abs(t)*1000. / getTickFrequency());
	system("pause");
	return 0;
}
