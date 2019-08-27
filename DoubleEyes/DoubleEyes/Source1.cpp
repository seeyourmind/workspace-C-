#include "stdafx.h"
#include <opencv2\imgproc\imgproc.hpp>//统计代码运行时间
#include "ProbeRecognition.h"
#include "UltrasoundImageFusion.h"
#include <thread>


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
void imreadTask(Mat *im, string file, Size imgsize) {
	*im = imread(file);
	resize(*im, *im, imgsize);
	return;
}
void recognitionTask(Mat pic, vector<Point2f> *vpf) {
	*vpf = ProbeRecognition::probeRecognition(pic);
	return;
}
int mainXanxus() {
	double t0 = (double)getTickCount();
	double t = (double)getTickCount();
	Mat picl, picr, ultrasoundImg;
	thread t1(imreadTask, &picl, "C:/Users/Fyzer/Desktop/图片1l.png", Size(400, 400));
	thread t2(imreadTask, &picr, "C:/Users/Fyzer/Desktop/图片1r.png", Size(400, 400));
	thread t3(imreadTask, &ultrasoundImg, "D:/document/ffmpeg/cs.png", Size(200, 200));
	t1.join(); t2.join(); t3.join();
	t -= (double)getTickCount();
	printf("thread imread execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	/*t = (double)getTickCount();
	Mat picl = imread("D:/UserCode/src/0.png");
	Mat picr = imread("D:/UserCode/src/1.png");
	Mat ultrasoundImg = imread("D:/UserCode/src/cs.png");
	t -= (double)getTickCount();
	printf("imread execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	Size imgResize(400, 400);
	resize(picl, picl, imgResize);
	resize(picr, picr, imgResize);*/

	t = (double)getTickCount();
	vector<Point2f> lktq_l, lktq_r;
	thread t4(recognitionTask, picl, &lktq_l);
	thread t5(recognitionTask, picr, &lktq_r);
	t4.join(); t5.join();
	/*vector<Point2f> lktq_l = ProbeRecognition::probeRecognition(picl);
	vector<Point2f> lktq_r = ProbeRecognition::probeRecognition(picr);*/
	lktq_l.pop_back();
	lktq_r.pop_back();
	t -= (double)getTickCount();
	printf("probeRecognition execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	cout << ProbeRecognition::angle << endl;
	t = (double)getTickCount();
	UltrasoundImageFusion::ultrasoundImageFusion(picl, picr, ultrasoundImg, lktq_l, lktq_r, ProbeRecognition::angle, 0.2);//
	t -= (double)getTickCount();
	printf("ultrasoundImageFusion execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	/**/
	t0 -= (double)getTickCount();
	printf("execution time = %gms\n", abs(t0)*1000. / getTickFrequency());
	system("pause");
	return 0;
}

int main00000(void) {
	cv::Mat src = cv::imread("C:/Users/Fyzer/Desktop/test.png");
	cv::imshow("src", src);
	double angle = -45;
	cv::Point2f center(src.cols / 2, src.rows / 2);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1);
	cv::Rect bbox = cv::RotatedRect(center, src.size(), angle).boundingRect();

	rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;

	cv::Mat dst;
	cv::warpAffine(src, dst, rot, bbox.size());
	cv::imshow("dst", dst);
	cv::waitKey(0);

	return 0;
}
