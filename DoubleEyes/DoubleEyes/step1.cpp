#include "stdafx.h"
#include "ProbeRecognition.h"
#include <algorithm>  // 求数组中最值

vector<Point2f> step1processing(Mat img, string flag) {
	//resize(img, img, Size(img.cols / 2, img.rows / 2));
	//imshow("img src " + flag, img);
	//waitKey(0);
	// 三通道图及初筛选
	Mat ir = Mat(Size(img.cols, img.rows), CV_8UC1);
	Mat ig = Mat(Size(img.cols, img.rows), CV_8UC1);
	Mat ib = Mat(Size(img.cols, img.rows), CV_8UC1);
	for (int i = 0; i < img.rows; ++i) {
		uchar* ip = img.ptr<uchar>(i);
		uchar* bp = ib.ptr<uchar>(i);
		uchar* gp = ig.ptr<uchar>(i);
		uchar* rp = ir.ptr<uchar>(i);
		for (int j = 0; j < img.cols; ++j) {
			bp[j] = ip[j * 3 + 0] < 100 ? 0 : ip[j * 3 + 0];
			gp[j] = ip[j * 3 + 1] < 100 ? 0 : ip[j * 3 + 1];
			rp[j] = ip[j * 3 + 2] < 100 ? 0 : ip[j * 3 + 2];
		}
	}
	///cout << "三通道图及初筛选完成" << endl;

	// 筛选
	for (int i = 0; i < ir.rows; ++i) {
		uchar* bp = ib.ptr<uchar>(i);
		uchar* gp = ig.ptr<uchar>(i);
		uchar* rp = ir.ptr<uchar>(i);
		for (int j = 0; j < ir.cols; ++j) {
			if (rp[j] != 0 && rp[j] - gp[j] < 10 && rp[j] - bp[j] < 20) {
				rp[j] = 255;
			}
			else {
				rp[j] = 0;
			}
		}
	}
	//imshow("i bw ir", ir);
	//waitKey(100);
	///cout << "筛选完成" << endl;

	// 轻微腐蚀
	Mat erodelement = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));//MORPH_ELLIPSE, MORPH_RECT, MORPH_CROSS
	erode(ir, ir, erodelement);
	//imshow("腐蚀操作", ir);
	//waitKey(100);
	//*
	// 轻微膨胀
	Mat dilatelement = getStructuringElement(MORPH_RECT, Size(5, 5));
	dilate(ir, ir, dilatelement);
	//imshow("膨胀操作", ir);
	//waitKey(100);
	//*/
	// 外接矩形
	Mat dst = Mat::zeros(ir.size(), CV_8UC3);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(ir, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));
	RNG rng(0);//随机数产生器
	int maxContoursSize = 0;
	int maxContoursIndex = 0;
	if (contours.size() <= 0) {
		//cout << flag << ": 未检测到探头" << endl;
		//imshow("img " + flag, img); 
		return vector<Point2f>();
		//return img;
	}
	for (int i = 0; i < contours.size(); ++i) {
		//绘制轮廓的最小外结矩形
		if (maxContoursSize < contourArea(contours[i])) {
			/*if (contourArea(contours[i]) <= 9000) {
				maxContoursSize = contourArea(contours[i]);
				maxContoursIndex = i;
				//cout << "max contour index: " << i << endl;
			}*/
			maxContoursSize = contourArea(contours[i]);
			maxContoursIndex = i;
			//cout << "inside max contour size: " << maxContoursSize << endl;
		}
		//cout << "max contour size: " << contourArea(contours[i]) << endl;
		/*drawContours(dst, contours, i, Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), 1, 8);
		imshow("i bw contours", dst);
		waitKey(0);*/
	}
	///cout << "外接矩形完成" << endl;

	// 绘制定位轮廓
	Mat img_copy;
	img.copyTo(img_copy);
	RotatedRect rect = minAreaRect(contours[maxContoursIndex]);
	if (rect.size.area()<50*50) {
		///cout << "无效rect" << endl;
		return vector<Point2f>();
		//return img;
	}
	Point2f P[4];
	vector<Point2f> returnPoints;
	rect.points(P);
	vector<Point> parray;
	///cout << "绘制定位轮廓完成" << endl;
	for (int i = 0; i < 4; i++) {
		line(img_copy, P[i], P[(i + 1) % 4], Scalar(255, 0, 0), 2, 8);
		parray.push_back(P[i]);
	}
	//cout << flag << endl;
	returnPoints.push_back(P[0]);
	//cout << P[0] << P[1] << P[2] << P[3] << endl;
	//cout << "angle:" << rect.angle << endl;
	///cout << "width:" << rect.size.width << endl;
	///cout << "height:" << rect.size.height << endl;
	///cout << "area:" << rect.size.area() << endl;
	//waitKey(0);
	//imshow("img" + flag, img_copy);
	//waitKey(1000);
	//return img_copy;
	
	// 提取定位ROI
	///Mat rot_mat = getRotationMatrix2D(rect.center, rect.angle, 1.0);
	Mat rot_img;
	// 计算旋转后输出的图形尺寸
	int rotated_width = ceil(img.rows*fabs(sin(rect.angle*PI / 180.)) + img.cols*fabs(cos(rect.angle*PI / 180.)));
	int rotated_height = ceil(img.cols*fabs(sin(rect.angle*PI / 180.)) + img.rows*fabs(cos(rect.angle*PI / 180.)));
	// 计算仿射变换矩阵
	Point2f imgCenter(img.cols / 2, img.rows / 2);
	Mat rotated_matrix = getRotationMatrix2D(imgCenter, rect.angle, 1.);
	// 防止切边，对平移矩阵进行修改
	rotated_matrix.at<double>(0, 2) += (rotated_width - img.cols) / 2;
	rotated_matrix.at<double>(1, 2) += (rotated_height - img.rows) / 2;
	// 应用仿射变换
	warpAffine(img, rot_img, rotated_matrix, Size(rotated_width,rotated_height));//原图旋转
	// 计算旋转后rect的中心
	double rect_center_x, rect_center_y;
	if (rect.angle < 0) {
		rect_center_x = (rect.center.x - imgCenter.x)*cos(abs(rect.angle)*PI / 180.) - (rect.center.y - imgCenter.y)*sin(abs(rect.angle)*PI / 180.) + imgCenter.x + (rotated_width - img.cols) / 2;
		rect_center_y = (rect.center.x - imgCenter.x)*sin(abs(rect.angle)*PI / 180.) + (rect.center.y - imgCenter.y)*cos(abs(rect.angle)*PI / 180.) + imgCenter.y + (rotated_height - img.rows) / 2;
	}
	else {
		rect_center_x = (rect.center.x - imgCenter.x)*cos(abs(rect.angle)*PI / 180.) + (rect.center.y - imgCenter.y)*sin(abs(rect.angle)*PI / 180.) + imgCenter.x + (rotated_width - img.cols) / 2;
		rect_center_y = (rect.center.x - imgCenter.x)*sin(abs(rect.angle)*PI / 180.) - (rect.center.y - imgCenter.y)*cos(abs(rect.angle)*PI / 180.) + imgCenter.y + (rotated_height - img.rows) / 2;
	}
	
	circle(rot_img, Point2f(rect_center_x, rect_center_y), 4, Scalar(0, 0, 255));
	//imshow("rot_img", rot_img);
	//waitKey(100);
	
	Mat rectROI = rot_img(Rect(rect_center_x - (rect.size.width / 2), rect_center_y - (rect.size.height / 2), rect.size.width, rect.size.height));
	//imshow("roi", rectROI);
	//waitKey(100);
	//cout << "提取定位ROI完成" << endl;

	// 确定黑色标记位置
	Mat roi_bw = Mat::zeros(rectROI.size(), CV_8UC1);
	for (int i = 0; i < roi_bw.rows; ++i) {
		uchar *ptr = roi_bw.ptr<uchar>(i);
		uchar *ptr0 = rectROI.ptr<uchar>(i);
		for (int j = 0; j < roi_bw.cols; ++j) {
			if (ptr0[j * 3 + 0] < 40 && ptr0[j * 3 + 1] < 40 && ptr0[j * 3 + 2] < 40) {
				ptr[j] = 255;
			}
		}
	}
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(roi_bw, roi_bw, MORPH_DILATE, element);
	///imshow("roi bw", roi_bw);
	Mat roidst = Mat::zeros(rectROI.size(), CV_8UC3);
	vector<vector<Point>> roicontours;
	vector<Vec4i> roihierarchy;
	findContours(roi_bw, roicontours, roihierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));
	int maxRoiContoursIndex = -1;
	for (int i = 0; i < roicontours.size(); ++i) {
		//绘制轮廓的最小外结矩形
		//string text = to_string(contourArea(roicontours[i]));
		//cout << "area: " << text << " length: " << arcLength(roicontours[i], true) << endl;
		//putText(roidst, text, roicontours[i][0], FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0,0,255), 1, 1);
		float iarea = contourArea(roicontours[i]);
		float ilength = arcLength(roicontours[i], true);
		if (iarea > 50 && iarea < 200 && ilength>40 && ilength < 80) {
			//cout << "area: " << iarea << " length: " << ilength << endl;
			drawContours(roidst, roicontours, i, Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), 2, 8);
			maxRoiContoursIndex = i;
			//imshow("roi bw", roidst);
			//waitKey(10);
		}
	}
	///cout << "确定黑色标记位置" << endl;

	int jkjkj = 0;
	if (maxRoiContoursIndex != -1) {
		RotatedRect rect2 = minAreaRect(roicontours[maxRoiContoursIndex]);
		Point2f P2[4];
		rect2.points(P2);
		float xs[4] = { P2[0].x,P2[1].x ,P2[2].x ,P2[3].x };
		float ys[4] = { P2[0].y,P2[1].y ,P2[2].y ,P2[3].y };
		float minx = *min_element(xs, xs + 4);
		float maxx = *max_element(xs, xs + 4);
		float miny = *min_element(ys, ys + 4);
		float maxy = *max_element(ys, ys + 4);
		if (roi_bw.rows < roi_bw.cols) {
			if (roi_bw.rows - maxy > miny) {
				jkjkj = 1;
				//cout << "y检测到标记物，融合方向为探头下方一侧" << endl;
			}
			else {
				jkjkj = 2;
				//cout << "y检测到标记物，融合方向为探头上方一侧" << endl;
			}
		}
		else {
			if (roi_bw.cols - maxx < minx) {
				jkjkj = 3;
				//cout << "x检测到标记物，融合方向为探头下方一侧" << endl;
			}
			else {
				jkjkj = 4;
				//cout << "x检测到标记物，融合方向为探头上方一侧" << endl;
			}
		}
	}
	else {
		jkjkj = 0;
		//cout << "未检测到标记物，融合方向将以默认方式进行" << endl;
	}
	returnPoints.push_back(Point2f(rect.angle, jkjkj));
	return returnPoints;
	//*/
}
/*
Point3f step1processing(Mat img, string flag) {
//resize(img, img, Size(img.cols / 2, img.rows / 2));
//imshow("img src " + flag, img);
//waitKey(0);
// 三通道图及初筛选
Mat ir = Mat(Size(img.cols, img.rows), CV_8UC1);
Mat ig = Mat(Size(img.cols, img.rows), CV_8UC1);
Mat ib = Mat(Size(img.cols, img.rows), CV_8UC1);
for (int i = 0; i < img.rows; ++i) {
uchar* ip = img.ptr<uchar>(i);
uchar* bp = ib.ptr<uchar>(i);
uchar* gp = ig.ptr<uchar>(i);
uchar* rp = ir.ptr<uchar>(i);
for (int j = 0; j < img.cols; ++j) {
bp[j] = ip[j * 3 + 0] < 100 ? 0 : ip[j * 3 + 0];
gp[j] = ip[j * 3 + 1] < 100 ? 0 : ip[j * 3 + 1];
rp[j] = ip[j * 3 + 2] < 100 ? 0 : ip[j * 3 + 2];
}
}
///cout << "三通道图及初筛选完成" << endl;

// 筛选
for (int i = 0; i < ir.rows; ++i) {
uchar* bp = ib.ptr<uchar>(i);
uchar* gp = ig.ptr<uchar>(i);
uchar* rp = ir.ptr<uchar>(i);
for (int j = 0; j < ir.cols; ++j) {
if (rp[j] != 0 && rp[j] - gp[j] < 10 && rp[j] - bp[j] < 20) {
rp[j] = 255;
}
else {
rp[j] = 0;
}
}
}
//imshow("i bw ir", ir);
//waitKey(100);
///cout << "筛选完成" << endl;

// 轻微腐蚀
Mat erodelement = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));//MORPH_ELLIPSE, MORPH_RECT, MORPH_CROSS
erode(ir, ir, erodelement);
//imshow("腐蚀操作", ir);
//waitKey(100);
//*
// 轻微膨胀
Mat dilatelement = getStructuringElement(MORPH_RECT, Size(5, 5));
dilate(ir, ir, dilatelement);
//imshow("膨胀操作", ir);
//waitKey(100);
//*
// 外接矩形
Mat dst = Mat::zeros(ir.size(), CV_8UC3);
vector<vector<Point>> contours;
vector<Vec4i> hierarchy;
findContours(ir, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));
RNG rng(0);//随机数产生器
int maxContoursSize = 0;
int maxContoursIndex = 0;
if (contours.size() <= 0) {
	//cout << flag << ": 未检测到探头" << endl;
	//imshow("img " + flag, img);
	//return vector<Point2f>();
	return img;
}
for (int i = 0; i < contours.size(); ++i) {
	//绘制轮廓的最小外结矩形
	if (maxContoursSize < contourArea(contours[i])) {
		/*if (contourArea(contours[i]) <= 9000) {
		maxContoursSize = contourArea(contours[i]);
		maxContoursIndex = i;
		//cout << "max contour index: " << i << endl;
		}*
		maxContoursSize = contourArea(contours[i]);
		maxContoursIndex = i;
		//cout << "inside max contour size: " << maxContoursSize << endl;
	}
	//cout << "max contour size: " << contourArea(contours[i]) << endl;
	/*drawContours(dst, contours, i, Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), 1, 8);
	imshow("i bw contours", dst);
	waitKey(0);*
}
///cout << "外接矩形完成" << endl;

// 绘制定位轮廓
Mat img_copy;
img.copyTo(img_copy);
RotatedRect rect = minAreaRect(contours[maxContoursIndex]);
if (rect.size.area()<50 * 50) {
	///cout << "无效rect" << endl;
	//return vector<Point2f>();
	return img;
}
Point2f P[4];
vector<Point2f> returnPoints(P, P + 4);
rect.points(P);
vector<Point> parray;
///cout << "绘制定位轮廓完成" << endl;
for (int i = 0; i < 4; i++) {
	line(img_copy, P[i], P[(i + 1) % 4], Scalar(255, 0, 0), 2, 8);
	parray.push_back(P[i]);
}
//cout << flag << endl;
cout << P[0] << P[1] << P[2] << P[3] << endl;
cout << "angle:" << rect.angle << endl;
///cout << "width:" << rect.size.width << endl;
///cout << "height:" << rect.size.height << endl;
///cout << "area:" << rect.size.area() << endl;
//waitKey(0);
//imshow("img" + flag, img_copy);
//waitKey(1000);
return img_copy;
*
// 提取定位ROI
///Mat rot_mat = getRotationMatrix2D(rect.center, rect.angle, 1.0);
Mat rot_img;
// 计算旋转后输出的图形尺寸
int rotated_width = ceil(img.rows*fabs(sin(rect.angle*PI / 180.)) + img.cols*fabs(cos(rect.angle*PI / 180.)));
int rotated_height = ceil(img.cols*fabs(sin(rect.angle*PI / 180.)) + img.rows*fabs(cos(rect.angle*PI / 180.)));
// 计算仿射变换矩阵
Point2f imgCenter(img.cols / 2, img.rows / 2);
Mat rotated_matrix = getRotationMatrix2D(imgCenter, rect.angle, 1.);
// 防止切边，对平移矩阵进行修改
rotated_matrix.at<double>(0, 2) += (rotated_width - img.cols) / 2;
rotated_matrix.at<double>(1, 2) += (rotated_height - img.rows) / 2;
// 应用仿射变换
warpAffine(img, rot_img, rotated_matrix, Size(rotated_width,rotated_height));//原图旋转
// 计算旋转后rect的中心
double rect_center_x, rect_center_y;
if (rect.angle < 0) {
rect_center_x = (rect.center.x - imgCenter.x)*cos(abs(rect.angle)*PI / 180.) - (rect.center.y - imgCenter.y)*sin(abs(rect.angle)*PI / 180.) + imgCenter.x + (rotated_width - img.cols) / 2;
rect_center_y = (rect.center.x - imgCenter.x)*sin(abs(rect.angle)*PI / 180.) + (rect.center.y - imgCenter.y)*cos(abs(rect.angle)*PI / 180.) + imgCenter.y + (rotated_height - img.rows) / 2;
}
else {
rect_center_x = (rect.center.x - imgCenter.x)*cos(abs(rect.angle)*PI / 180.) + (rect.center.y - imgCenter.y)*sin(abs(rect.angle)*PI / 180.) + imgCenter.x + (rotated_width - img.cols) / 2;
rect_center_y = (rect.center.x - imgCenter.x)*sin(abs(rect.angle)*PI / 180.) - (rect.center.y - imgCenter.y)*cos(abs(rect.angle)*PI / 180.) + imgCenter.y + (rotated_height - img.rows) / 2;
}

circle(rot_img, Point2f(rect_center_x, rect_center_y), 4, Scalar(0, 0, 255));
//imshow("rot_img", rot_img);
//waitKey(100);

Mat rectROI = rot_img(Rect(rect_center_x - (rect.size.width / 2), rect_center_y - (rect.size.height / 2), rect.size.width, rect.size.height));
//imshow("roi", rectROI);
//waitKey(100);
cout << "提取定位ROI完成" << endl;

// 确定黑色标记位置
Mat roi_bw = Mat::zeros(rectROI.size(), CV_8UC1);
for (int i = 0; i < roi_bw.rows; ++i) {
uchar *ptr = roi_bw.ptr<uchar>(i);
uchar *ptr0 = rectROI.ptr<uchar>(i);
for (int j = 0; j < roi_bw.cols; ++j) {
if (ptr0[j * 3 + 0] < 40 && ptr0[j * 3 + 1] < 40 && ptr0[j * 3 + 2] < 40) {
ptr[j] = 255;
}
}
}
Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
morphologyEx(roi_bw, roi_bw, MORPH_DILATE, element);
///imshow("roi bw", roi_bw);
Mat roidst = Mat::zeros(rectROI.size(), CV_8UC3);
vector<vector<Point>> roicontours;
vector<Vec4i> roihierarchy;
findContours(roi_bw, roicontours, roihierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));
int maxRoiContoursIndex = -1;
for (int i = 0; i < roicontours.size(); ++i) {
//绘制轮廓的最小外结矩形
//string text = to_string(contourArea(roicontours[i]));
//cout << "area: " << text << " length: " << arcLength(roicontours[i], true) << endl;
//putText(roidst, text, roicontours[i][0], FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0,0,255), 1, 1);
float iarea = contourArea(roicontours[i]);
float ilength = arcLength(roicontours[i], true);
if (iarea > 50 && iarea < 200 && ilength>40 && ilength < 80) {
//cout << "area: " << iarea << " length: " << ilength << endl;
drawContours(roidst, roicontours, i, Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), 2, 8);
maxRoiContoursIndex = i;
//imshow("roi bw", roidst);
//waitKey(10);
}
}
///cout << "确定黑色标记位置" << endl;

int jkjkj = 0;
if (maxRoiContoursIndex != -1) {
RotatedRect rect2 = minAreaRect(roicontours[maxRoiContoursIndex]);
Point2f P2[4];
rect2.points(P2);
float xs[4] = { P2[0].x,P2[1].x ,P2[2].x ,P2[3].x };
float ys[4] = { P2[0].y,P2[1].y ,P2[2].y ,P2[3].y };
float minx = *min_element(xs, xs + 4);
float maxx = *max_element(xs, xs + 4);
float miny = *min_element(ys, ys + 4);
float maxy = *max_element(ys, ys + 4);
if (roi_bw.rows < roi_bw.cols) {
if (roi_bw.rows - maxy > miny) {
jkjkj = 1;
//cout << "y检测到标记物，融合方向为探头下方一侧" << endl;
}
else {
jkjkj = 2;
//cout << "y检测到标记物，融合方向为探头上方一侧" << endl;
}
}
else {
if (roi_bw.cols - maxx < minx) {
jkjkj = 3;
//cout << "x检测到标记物，融合方向为探头下方一侧" << endl;
}
else {
jkjkj = 4;
//cout << "x检测到标记物，融合方向为探头上方一侧" << endl;
}
}
}
else {
jkjkj = 0;
//cout << "未检测到标记物，融合方向将以默认方式进行" << endl;
}
return returnPoints;
*
}
*/
/*
int main(){
	Mat img = imread("frameL500.jpg");
	Mat resultm = step1processing(img, "flag");
	imshow("result", resultm);
	waitKey(0);
	system("puase");
	return 0;
}*/
