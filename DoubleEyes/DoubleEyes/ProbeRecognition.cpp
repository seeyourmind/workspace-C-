#include "stdafx.h"
#include "ProbeRecognition.h"

// ��򷨷ָ�ͼ��
Mat ProbeRecognition::getProbeMarkUseOTSU(Mat img) {
	Mat gray, dst;
	cvtColor(img, gray, CV_BGR2GRAY);
	threshold(gray, dst, 0, 255, CV_THRESH_OTSU);/*

	imshow("�����䷽��ָ�", dst);
	waitKey(1000);*/
	return dst;
}

// kmeans�ָ�ͼ��
Mat ProbeRecognition::getProbeMarkUseKmeans(Mat pic) {
	const int MAX_CLUSTERS = 5;//��������
	Vec3b colorTab[] = {//�������ɫ
		Vec3b(255,255,255),
		Vec3b(0,0,0),
		Vec3b(255,100,100),
		Vec3b(255,0,255),
		Vec3b(0,255,255)
	};
	Mat data, labels;
	//cvtColor(pic, pic, CV_BGR2GRAY);
	//cvtColor(pic, pic, CV_BGR2Lab);
	// �������������ÿһ�����ص��Ӧ��B\G\Rֵ
	for (int i = 0; i < pic.rows; i++) {
		for (int j = 0; j < pic.cols; j++) {
			Vec3b point = pic.at<Vec3b>(i, j);
			Mat tmp = (Mat_<float>(1, 3) << point[0], point[1], point[2]);
			data.push_back(tmp);
		}
	}
	// cout << "data size:" << data.size << ";data rows:" << data.rows << ";data cols:" << data.cols << "data channel:" << data.channels() << endl;
	// kmeas
	/**
	 * kmeans(InputArray data, int K, InputOutputArray bestLabels, TermCriteria criteria, int attempts, int flags, OutputArray centers=noArray())
	 * data:��Ҫ�Զ���������ݣ�һ����һ��Mat�������͵ľ���ÿ��Ϊһ��������
	   k:ȡ�ɼ��࣬�ȽϹؼ���һ��������
	   bestLabels:���ص������,�������֡�
	   criteria:�㷨�����ı�׼����ȡ�������ȵĵ���������
	   attempts:�ж�ĳ������Ϊĳ��������پ������������ֵΪ3ʱ����ĳ����������3�ζ�Ϊͬһ���࣬��ȷ��������
	   flags:ȷ�����ĵļ��㷽ʽ��������ֵ��ѡ��KMEANS_RANDOM_CENTERS ��ʾ�����ʼ�����ġ�
	                                         KMEANS_PP_CENTERS ��ʾ��kmeans++�㷨����ʼ������
											 KMEANS_USE_INITIAL_LABELS ��ʾ��һ�ξ���ʱ���û�������ֵ��ʼ�����࣬���漸�εľ��࣬���Զ�ȷ�����ġ�
	   centers: ������ʼ�����ĵġ���ǰһ��flags������ѡ���йء����ѡ��KMEANS_RANDOM_CENTERS�����ʼ�����ģ������������ʡ�ԡ�
	 */
	kmeans(data, 2, labels, TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1.0), 3, KMEANS_RANDOM_CENTERS);
	int n = 0;
	// ��ʾ������
	for (int i = 0; i < pic.rows; i++) {
		for (int j = 0; j < pic.cols; j++) {
			int clusterIdx = labels.at<int>(n);
			pic.at<Vec3b>(i, j) = colorTab[clusterIdx];
			n++;
		}
	}/*
	imshow("kmeans��������ָ�", pic);
	waitKey(1000);*/
	return pic;
}

// ��̬ѧ����Ԥ����ͼ��ʹ̽ͷλ��ͻ����ʾ
Mat ProbeRecognition::morphologicalTreatment(Mat img, int type, Size size) {
	Mat out;
	Mat element = getStructuringElement(MORPH_RECT, size);//������ξ����
	//morphologyEx(img, out, MORPH_GRADIENT, element);//��̬ѧ�ݶ�
	//morphologyEx(img, out, MORPH_TOPHAT, element);//��ñ
	morphologyEx(img, out, type, element);/*/��ñ
	imshow("��̬ѧ����:", out);
	waitKey(1000);*/
	return out;
}

// ������ȡ��ȷ��̽ͷ����
vector<Point2f> ProbeRecognition::getProbeMarkUseContours(Mat src) {
	vector<Point2f> returnPoints;
	Mat dst = Mat::zeros(src.size(), CV_8UC3);
	if (src.channels() >= 3) {
		blur(src, src, Size(3, 3));//ƽ���������ֵ
		cvtColor(src, src, COLOR_BGR2GRAY);//BGRת�Ҷ�ͼ
	}
	/**
	 * ��Ե���
	   src:CvArr*: ����ͼ�񣬱���Ϊ��ͨ���Ҷ�ͼ��
	   src:CvArr*: �����Եͼ�񣬶�ֵͼ��
	   20:double: ��ֵ��С��ֵ���Ʊ�Ե����
	   80:double: ��ֵ������ֵ����ǿ��Ե�ĳ�ʼ�ָ�
				  ���ص��ݶȴ��ڴ���ֵ����Ϊ�Ǳ�Ե��С��С��ֵ������
	   3:int: Sobel���ӵĴ�С��Ĭ��3
	*/
	Canny(src, src, 100, 150, 3, false);/*/Canny����
	imshow("canny result", src);
	waitKey(0);*/
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
	findContours(src, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

	RNG rng(0);//�����������
	int maxContoursSize = 0;
	int maxContoursIndex = 0;
	for (int i = 0; i < contours.size(); i++) {
		// ����0-255֮�������������������ȡ������ɫ
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		/**
		 * ��������
		   dst:InputOutputArray:���ͼ��
		   contours:OutputArrayOfArrays:�ҵ���ȫ����������
		   i:int:����������
		   color:const Scalar:������ɫ
		   2:int thickness:�����߿�
		   8:int lineType:�ߵ����ͣ�Ĭ��8
		   hierarchy:InputArray:���˽ṹͼ
		   0:int maxlevel:��������0ֻ���Ƶ�ǰ��1��ʾ���Ƶ�ǰ������Ƕ��������
		   Point:offset:����λ��
		*/
		if (contourArea(contours[i])>200) {
			if (maxContoursSize < contours[i].size()) {
				maxContoursSize = contours[i].size();
				maxContoursIndex = i;
			}
		}
		/*
		drawContours(dst, contours, i, color, CV_FILLED, 8, hierarchy, 0, Point(0, 0));
		imshow("contours", dst);
		waitKey(50);*/
	}
	//cout << "max contour size:" << maxContoursSize << ";max contour index:" << maxContoursIndex << endl << contours[maxContoursIndex] << endl;
	//������������С������
	RotatedRect rect = minAreaRect(contours[maxContoursIndex]);
	cout << "rotate rect angle:" << rect.angle << endl << "max contours index is:" << maxContoursIndex << endl << "max contours size is:" << contours[maxContoursIndex].size() << endl;
	Point2f P[4];
	rect.points(P);
	for (int j = 0; j <= 3; j++) {
		line(dst, P[j], P[(j + 1) % 4], Scalar(255, 255, 255), 2);
		returnPoints.push_back(P[j]);
	}

	return returnPoints;
}

void getStraightLineUseHough(Mat image) {
	Mat result;
	cvtColor(image, result, CV_BGR2GRAY);
	Mat contours;
	Canny(result, contours, 125, 350);/*
	imshow("canny", contours);
	waitKey();*/
	//Hough�任���ֱ��
	vector<Vec2f> lines;
	HoughLines(contours, lines, 1, CV_PI / 180, 100);
	cout << "lines size:" << lines.size() << endl;
	//����ÿ����
	vector<Vec2f>::const_iterator it = lines.begin();
	while (it != lines.end()) {
		float rho = (*it)[0];//����rho
		float theta = (*it)[1];//�Ƕ�theta
		if (theta<CV_PI / 4. || theta>3.*CV_PI / 4.) {//��ֱ��
			//�����һ�еĽ���
			Point pt1(rho / cos(theta), 0);
			//�������һ�еĽ���
			Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);
			//���ư���
			line(image, pt1, pt2, Scalar(255), 1);
			cout << "����ֱ��" << endl;
		}
		else {//ˮƽ��
			//�����һ�еĽ���
			Point pt1(0, rho / sin(theta));
			//�������һ�еĽ���
			Point pt2(result.cols, (rho - contours.cols*cos(theta)) / sin(theta));
			//���ư���
			line(image, pt1, pt2, Scalar(255), 1);
			cout << "��ˮƽ��:" << endl << "��һ�еĽ��㣺" << pt1 << ";�ڶ��еĽ��㣺" << pt2 << endl;
		}
		++it;
		imshow("hough", image);
		waitKey(10);
	}
	
}

//����ӿ�
vector<Point2f> ProbeRecognition::probeRecognition(Mat img) {
	vector<Point2f> probePoints;
	if (img.empty()) { cout << "can not load image!" << endl; }
	else {/*
		imshow("ԭʼͼ��", img);
		waitKey(1000);*/
		ProbeRecognition pr;
		Mat out = pr.morphologicalTreatment(img, MORPH_BLACKHAT, Size(15,15));
		Mat out2 = pr.getProbeMarkUseKmeans(out);
		/*Mat testT = pr.morphologicalTreatment(out2, MORPH_ERODE, Size(3, 3));
		imshow(to_string(rand() % 100) + "test t", testT);
		waitKey(0);*/
		probePoints = pr.getProbeMarkUseContours(out);
	}
	return probePoints;
}

/*int main() {
	//Mat pic = imread("C:/Users/Fyzer/Desktop/probe_angle_2_left.png");
	Mat pic = imread("C:/Users/Fyzer/Desktop/probe_angle_left.png");
	//Mat pic = imread("C:/Users/Fyzer/Desktop/probe_right.png");
	vector<Point2f> keyPoints = ProbeRecognition::probeRecognition(pic);
	cout << "get the key points of porbe is:" << endl << keyPoints[0] << ";" << keyPoints[1] << ";" << keyPoints[2] << ";" << keyPoints[3] << endl;
	system("pause");
	return 0;
}*/