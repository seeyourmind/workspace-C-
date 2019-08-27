#include "stdafx.h"
#include "ProbeRecognition.h"

double ProbeRecognition::angle = 0;

Mat get255(Mat img) {
	double t = (double)getTickCount();
	if (img.channels() != 1) perror("please input a gray image!");
	for (int i = 0; i < img.rows; ++i) {
		uchar* p = img.ptr<uchar>(i);
		for (int j = 0; j < img.cols; ++j) {
			if (p[j] != 255) p[j] = 0;
		}
	}
	t -= (double)getTickCount();
	return img;
}
Mat getcombine255(Mat imgsrc, Mat img1, Mat img2) {
	double t = (double)cvGetTickCount();
	if (imgsrc.channels() != 1 || img1.channels() != 1 || img2.channels() != 1) perror("please mark sure you input the gray image for each parameter!");
	if (imgsrc.rows != img1.rows || img1.rows != img2.rows || imgsrc.cols != img1.cols || img1.cols != img2.cols) perror("please mark sure you input parameters have same size!");
	for (int i = 0; i < imgsrc.rows; ++i) {
		uchar* ps = imgsrc.ptr<uchar>(i);
		uchar* p1 = img1.ptr<uchar>(i);
		uchar* p2 = img2.ptr<uchar>(i);
		for (int j = 0; j < imgsrc.cols; ++j) {
			if (ps[j] == 255 && p1[j] != 255 && p2[j] != 255) ps[j] = 255;
			else ps[j] = 0;
		}
	}
	t -= (double)getTickCount();
	return imgsrc;
}

// 借助RGB通道差异
Mat getProbeMarkUseRGB(Mat img) {
	if (img.channels() != 3) perror("please input a RGB image!");
	Mat ir = Mat(Size(img.cols, img.rows), CV_8UC1);
	Mat ig = Mat(Size(img.cols, img.rows), CV_8UC1);
	Mat ib = Mat(Size(img.cols, img.rows), CV_8UC1);

	for (int i = 0; i < img.rows; ++i) {
		uchar* ip = img.ptr<uchar>(i);
		uchar* bp = ib.ptr<uchar>(i);
		uchar* gp = ig.ptr<uchar>(i);
		uchar* rp = ir.ptr<uchar>(i);
		for (int j = 0; j < img.cols; ++j) {
			bp[j] = ip[j * 3 + 0];
			gp[j] = ip[j * 3 + 1];
			rp[j] = ip[j * 3 + 2];
		}
	}
	Mat ir255, ig255, ib255, ic255;
	ir255 = get255(ir);
	ig255 = get255(ig);
	ib255 = get255(ib);

	ic255 = getcombine255(ig255, ir255, ib255);
	
	return ig;
}

// 轮廓提取，确定探头区域
vector<Point2f> getProbeMarkUseContours(Mat src) {
	Mat dst = Mat::zeros(src.size(), CV_8UC3);
	if (src.channels() >= 3) {
		blur(src, src, Size(3, 3));//平滑处理，求均值
		cvtColor(src, src, COLOR_BGR2GRAY);//BGR转灰度图
		/**
		* 边缘检测
		src:CvArr*: 输入图像，必须为单通道灰度图像
		src:CvArr*: 输出边缘图像，二值图像
		20:double: 阈值，小阈值控制边缘连接
		80:double: 阈值，大阈值控制强边缘的初始分割
		像素的梯度大于大阈值则认为是边缘，小于小阈值则被抛弃
		3:int: Sobel算子的大小，默认3
		*/
		Canny(src, src, 100, 150, 3, false);//Canny算子
	}
	
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	/**
	 * 提取轮廓
	   src:InputOutputArray:输入8bit二值图像，0不变，非0为1
	   contours:OutputArrayOfArrays:输出找到的轮廓对象
	   hierarchy:OutputArray:图像的拓扑结构，分别表示后一个轮廓，前一个轮廓，父轮廓，内嵌轮廓的索引
	   RETR_TREE:int:轮廓返回的模式：RETR_TREE:提取所有轮廓并重新建立网状轮廓结构
	   Point(): 轮廓像素的位移，默认没有位移(0,0)
	*/
	double t = (double)getTickCount();
	findContours(src, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

	//RNG rng(0);//随机数产生器
	int maxContoursSize = 0;
	int maxContoursIndex = 0;
	for (int i = 0; i < contours.size(); ++i) {
		// 产生0-255之间的随机数，用来随机获取轮廓颜色
		if (arcLength(contours[i], false)>200) {
			if (maxContoursSize < contours[i].size()) {
				maxContoursSize = contours[i].size();
				maxContoursIndex = i;
			}
		}
	}
	//绘制轮廓的最小外结矩形
	RotatedRect rect = minAreaRect(contours[maxContoursIndex]);
	ProbeRecognition::angle = rect.angle;
	Point2f P[4];
	rect.points(P);
	vector<Point2f> returnPoints(P, P + 4);
	
	return returnPoints;
}

//对外接口
vector<Point2f> ProbeRecognition::probeRecognition(Mat img) {
	//double t = (double)getTickCount();
	vector<Point2f> probePoints;
	if (img.empty()) { cout << "can not load image!" << endl; }
	else {
		Mat testT = getProbeMarkUseRGB(img);
		probePoints = getProbeMarkUseContours(testT);
	}
	return probePoints;
}
///*
int main9527() {
	double t = (double)getTickCount();
	double t0 = t;
	Mat pic = imread("C:/Users/Fyzer/Desktop/图片1l.png");
	resize(pic, pic, Size(400, 400));
	t -= (double)getTickCount();
	printf("read image execution time = %gms\n", abs(t)*1000. / getTickFrequency());
	
	t = (double)getTickCount();
	vector<Point2f> keyPoints = ProbeRecognition::probeRecognition(pic);
	t -= (double)getTickCount();
	printf("probe recognition execution time = %gms\n", abs(t)*1000. / getTickFrequency());
	cout << keyPoints << endl;
	system("pause");
	return 0;
}
