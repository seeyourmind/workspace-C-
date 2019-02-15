#include "stdafx.h"
#include "ProbeRecognition.h"

// 大津法分割图像
Mat ProbeRecognition::getProbeMarkUseOTSU(Mat img) {
	Mat gray, dst;
	cvtColor(img, gray, CV_BGR2GRAY);
	threshold(gray, dst, 0, 255, CV_THRESH_OTSU);/*

	imshow("最大类间方差法分割", dst);
	waitKey(1000);*/
	return dst;
}

// kmeans分割图像
Mat ProbeRecognition::getProbeMarkUseKmeans(Mat pic) {
	const int MAX_CLUSTERS = 5;//最大聚类数
	Vec3b colorTab[] = {//各类填充色
		Vec3b(255,255,255),
		Vec3b(0,0,0),
		Vec3b(255,100,100),
		Vec3b(255,0,255),
		Vec3b(0,255,255)
	};
	Mat data, labels;
	//cvtColor(pic, pic, CV_BGR2GRAY);
	//cvtColor(pic, pic, CV_BGR2Lab);
	// 构造聚类样本，每一个像素点对应的B\G\R值
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
	 * data:需要自动聚类的数据，一般是一个Mat。浮点型的矩阵，每行为一个样本。
	   k:取成几类，比较关键的一个参数。
	   bestLabels:返回的类别标记,整型数字。
	   criteria:算法结束的标准，获取期望精度的迭代最大次数
	   attempts:判断某个样本为某个类的最少聚类次数，比如值为3时，则某个样本聚类3次都为同一个类，则确定下来。
	   flags:确定簇心的计算方式。有三个值可选：KMEANS_RANDOM_CENTERS 表示随机初始化簇心。
	                                         KMEANS_PP_CENTERS 表示用kmeans++算法来初始化簇心
											 KMEANS_USE_INITIAL_LABELS 表示第一次聚类时用用户给定的值初始化聚类，后面几次的聚类，则自动确定簇心。
	   centers: 用来初始化簇心的。与前一个flags参数的选择有关。如果选择KMEANS_RANDOM_CENTERS随机初始化簇心，则这个参数可省略。
	 */
	kmeans(data, 2, labels, TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1.0), 3, KMEANS_RANDOM_CENTERS);
	int n = 0;
	// 显示聚类结果
	for (int i = 0; i < pic.rows; i++) {
		for (int j = 0; j < pic.cols; j++) {
			int clusterIdx = labels.at<int>(n);
			pic.at<Vec3b>(i, j) = colorTab[clusterIdx];
			n++;
		}
	}/*
	imshow("kmeans方法二类分割", pic);
	waitKey(1000);*/
	return pic;
}

// 形态学处理，预处理图像，使探头位置突出显示
Mat ProbeRecognition::morphologicalTreatment(Mat img, int type, Size size) {
	Mat out;
	Mat element = getStructuringElement(MORPH_RECT, size);//定义矩形卷积核
	//morphologyEx(img, out, MORPH_GRADIENT, element);//形态学梯度
	//morphologyEx(img, out, MORPH_TOPHAT, element);//顶帽
	morphologyEx(img, out, type, element);/*/黑帽
	imshow("形态学处理:", out);
	waitKey(1000);*/
	return out;
}

// 轮廓提取，确定探头区域
vector<Point2f> ProbeRecognition::getProbeMarkUseContours(Mat src) {
	vector<Point2f> returnPoints;
	Mat dst = Mat::zeros(src.size(), CV_8UC3);
	if (src.channels() >= 3) {
		blur(src, src, Size(3, 3));//平滑处理，求均值
		cvtColor(src, src, COLOR_BGR2GRAY);//BGR转灰度图
	}
	/**
	 * 边缘检测
	   src:CvArr*: 输入图像，必须为单通道灰度图像
	   src:CvArr*: 输出边缘图像，二值图像
	   20:double: 阈值，小阈值控制边缘连接
	   80:double: 阈值，大阈值控制强边缘的初始分割
				  像素的梯度大于大阈值则认为是边缘，小于小阈值则被抛弃
	   3:int: Sobel算子的大小，默认3
	*/
	Canny(src, src, 100, 150, 3, false);/*/Canny算子
	imshow("canny result", src);
	waitKey(0);*/
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
	findContours(src, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

	RNG rng(0);//随机数产生器
	int maxContoursSize = 0;
	int maxContoursIndex = 0;
	for (int i = 0; i < contours.size(); i++) {
		// 产生0-255之间的随机数，用来随机获取轮廓颜色
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		/**
		 * 绘制轮廓
		   dst:InputOutputArray:输出图像
		   contours:OutputArrayOfArrays:找到的全部轮廓对象
		   i:int:轮廓索引号
		   color:const Scalar:绘制颜色
		   2:int thickness:绘制线宽
		   8:int lineType:线的类型，默认8
		   hierarchy:InputArray:拓扑结构图
		   0:int maxlevel:最大层数（0只绘制当前，1表示绘制当前及其内嵌的轮廓）
		   Point:offset:轮廓位移
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
	//绘制轮廓的最小外结矩形
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
	//Hough变换检测直线
	vector<Vec2f> lines;
	HoughLines(contours, lines, 1, CV_PI / 180, 100);
	cout << "lines size:" << lines.size() << endl;
	//绘制每条线
	vector<Vec2f>::const_iterator it = lines.begin();
	while (it != lines.end()) {
		float rho = (*it)[0];//距离rho
		float theta = (*it)[1];//角度theta
		if (theta<CV_PI / 4. || theta>3.*CV_PI / 4.) {//垂直线
			//线与第一行的交点
			Point pt1(rho / cos(theta), 0);
			//线与最后一行的交点
			Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);
			//绘制白线
			line(image, pt1, pt2, Scalar(255), 1);
			cout << "画垂直线" << endl;
		}
		else {//水平线
			//线与第一列的交点
			Point pt1(0, rho / sin(theta));
			//线与最后一列的交点
			Point pt2(result.cols, (rho - contours.cols*cos(theta)) / sin(theta));
			//绘制白线
			line(image, pt1, pt2, Scalar(255), 1);
			cout << "画水平线:" << endl << "第一列的交点：" << pt1 << ";第二列的交点：" << pt2 << endl;
		}
		++it;
		imshow("hough", image);
		waitKey(10);
	}
	
}

//对外接口
vector<Point2f> ProbeRecognition::probeRecognition(Mat img) {
	vector<Point2f> probePoints;
	if (img.empty()) { cout << "can not load image!" << endl; }
	else {/*
		imshow("原始图像", img);
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