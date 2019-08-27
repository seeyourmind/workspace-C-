#include "stdafx.h"

Mat roateFusion(Mat dst, Mat src, double angle) {
	Point2f center(dst.cols/2, dst.rows/2);
	Mat rot = getRotationMatrix2D(center, angle, 1);//旋转矩阵
	Rect bbox = RotatedRect(center, dst.size(), angle).boundingRect();//旋转后矩阵大小
	
	rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;

	Mat dst0;
	warpAffine(src, dst0, rot, bbox.size());
	imshow("dst", dst0);
	waitKey(0);

	return dst0;
}