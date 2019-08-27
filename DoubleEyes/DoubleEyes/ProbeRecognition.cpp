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

// ����RGBͨ������
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

// ������ȡ��ȷ��̽ͷ����
vector<Point2f> getProbeMarkUseContours(Mat src) {
	Mat dst = Mat::zeros(src.size(), CV_8UC3);
	if (src.channels() >= 3) {
		blur(src, src, Size(3, 3));//ƽ���������ֵ
		cvtColor(src, src, COLOR_BGR2GRAY);//BGRת�Ҷ�ͼ
		/**
		* ��Ե���
		src:CvArr*: ����ͼ�񣬱���Ϊ��ͨ���Ҷ�ͼ��
		src:CvArr*: �����Եͼ�񣬶�ֵͼ��
		20:double: ��ֵ��С��ֵ���Ʊ�Ե����
		80:double: ��ֵ������ֵ����ǿ��Ե�ĳ�ʼ�ָ�
		���ص��ݶȴ��ڴ���ֵ����Ϊ�Ǳ�Ե��С��С��ֵ������
		3:int: Sobel���ӵĴ�С��Ĭ��3
		*/
		Canny(src, src, 100, 150, 3, false);//Canny����
	}
	
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	/**
	 * ��ȡ����
	   src:InputOutputArray:����8bit��ֵͼ��0���䣬��0Ϊ1
	   contours:OutputArrayOfArrays:����ҵ�����������
	   hierarchy:OutputArray:ͼ������˽ṹ���ֱ��ʾ��һ��������ǰһ������������������Ƕ����������
	   RETR_TREE:int:�������ص�ģʽ��RETR_TREE:��ȡ�������������½�����״�����ṹ
	   Point(): �������ص�λ�ƣ�Ĭ��û��λ��(0,0)
	*/
	double t = (double)getTickCount();
	findContours(src, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

	//RNG rng(0);//�����������
	int maxContoursSize = 0;
	int maxContoursIndex = 0;
	for (int i = 0; i < contours.size(); ++i) {
		// ����0-255֮�������������������ȡ������ɫ
		if (arcLength(contours[i], false)>200) {
			if (maxContoursSize < contours[i].size()) {
				maxContoursSize = contours[i].size();
				maxContoursIndex = i;
			}
		}
	}
	//������������С������
	RotatedRect rect = minAreaRect(contours[maxContoursIndex]);
	ProbeRecognition::angle = rect.angle;
	Point2f P[4];
	rect.points(P);
	vector<Point2f> returnPoints(P, P + 4);
	
	return returnPoints;
}

//����ӿ�
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
	Mat pic = imread("C:/Users/Fyzer/Desktop/ͼƬ1l.png");
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
