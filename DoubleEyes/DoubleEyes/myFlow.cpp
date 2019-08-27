#include "stdafx.h"
#include "myFlow.h"

// 1、根据视频图像提取探头位置关键点--像素坐标系
void getProbeLocation(Mat img);
// 2、关键点坐标像素坐标系--相机坐标系转换
vector<Point3f> getProbe3DLocation(vector<Point2i> points) {
	cout << "get probe 3d location is begining!!!" << endl;
	
	vector<Point3f> points3D;
	for (int i = 0; i < points.size();) {
		// 计算视差--图像坐标系
		float delta_x = DX*abs(points[i].y - points[i + 1].y);
		cout << "delt_x:" << delta_x << endl;
		// 计算三维坐标--相机坐标系
		Point3f pl = Point3f(BASELINE*DX*(points[i].x - U0) / delta_x, BASELINE*DY*(points[i].y - V0) / delta_x, BASELINE*FOCUS / delta_x);
		Point3f pr = Point3f(BASELINE*DX*(points[i + 1].x - U0) / delta_x, BASELINE*DY*(points[i + 1].y - V0) / delta_x, BASELINE*FOCUS / delta_x);
		points3D.push_back(pl);
		//points3D.push_back(pr);
		i += 2;
	}
	return points3D;
}
// 3、确定扫描平面--相机坐标系
MatrixXf getScanPanel(Mat keyPoints) {
	return getFactor(keyPoints);
}
// 求平面与垂直平面间的夹角
double getSinAlpha(MatrixXf factor) {
	double cosa = factor(2) / sqrt(pow(factor(0), 2) + pow(factor(1), 2) + pow(factor(2), 2));
	cout << "acos a:" << acos(cosa) << endl;
	return sin(acos(cosa));
}
// 5、计算超声图像的深度--相机坐标系
Mat getUltrasoundZ(Size ultrimg, MatrixXf factors) {
	cout << "get ultrasound z is begining!!!" << endl;
	Mat imgz = Mat::zeros(ultrimg.height, ultrimg.width, CV_32FC1);
	for (int i = 0; i < ultrimg.width; i++) {
		for (int j = 0; j < ultrimg.height; j++) {
			double z = -(ONEPLEX*(factors(0) * i + factors(1) * j) + factors(3)) / factors(2);
			imgz.at<float>(j, i) = z;
		}
	}
	return imgz;
}
Mat getUltrasoundZ(Size ultrimg, double sina) {
	cout << "get ultrasound z is begining!!!" << endl;
	Mat imgz = Mat::zeros(ultrimg.height, ultrimg.width, CV_32FC1);
	for (int i = 0; i < ultrimg.width; i++) {
		for (int j = 0; j < ultrimg.height; j++) {
			imgz.at<float>(j, i) = (i + 1)*DX*sina;
		}
	}
	return imgz;
}
// 6、根据深度计算超声图像的视差--像素坐标系
Mat getUlltrasoundParallax(Mat deeps) {
	cout << "get ultrasound parallax is begining!!!" << endl;
	Mat parallaxs = Mat::zeros(deeps.rows, deeps.cols, CV_16SC1);
	for (int i = 0; i < deeps.rows; i++) {
		for (int j = 0; j < deeps.cols; j++) {
			parallaxs.at<short>(i, j) = short((BASELINE*FOCUS) / (DX*deeps.at<float>(i, j)));
		}
	}
	return parallaxs;
}
Mat getUlltrasoundParallax(Mat deep, double focus, double baseline) {
	Mat disparity = Mat::zeros(deep.rows, deep.cols, CV_16SC1);
	for (int i = 0; i < deep.rows; i++) {
		for (int j = 0; j < deep.cols; j++) {
			disparity.at<short>(i, j) = short((focus*baseline) / deep.at<float>(i, j));
		}
	}
	//cout << disparity << endl;
	return disparity;
}

// 7、图像融合
Mat imageFuse(Mat src, Mat fuse, Mat parallaxMat, Point2i Ouv, double alpha) {
	for (int i = 0; i < fuse.rows; i++) {
		for (int j = 0; j < fuse.cols; j++) {
			int parallax = (int)parallaxMat.at<short>(i, j);
			src.at<Vec3b>(Ouv.y+i, Ouv.x+parallax+j) = alpha*src.at<Vec3b>(Ouv.y + i, Ouv.x + parallax + j) + (1 - alpha)*fuse.at<Vec3b>(i, j);
		}
	}
	return src;
}

// 像素坐标转图像坐标
Mat plex2image(Mat img)
{
	int u0 = (int)(img.cols / 2);
	int v0 = (int)(img.rows / 2);
	Mat image = Mat::zeros(img.rows, img.cols, CV_32FC2);

	for (int i = 0; i < img.cols; i++) {
		for (int j = 0; j < img.rows; j++) {
			image.at<Vec2f>(i, j)[0] = DX*(i - u0);
			image.at<Vec2f>(i, j)[1] = DY*(j - v0);
		}
	}
	return image;
}

// 求各点视差
Mat getDisparity(Mat imgL, Mat imgR)
{
	return imgL;
}

// 求目标点的三维坐标
void getKeyPoints() {

}

// 求目标平面
MatrixXf getTargetPanel(Mat keyPoints) {
	return getFactor(keyPoints);
}

// 计算超声图像坐标
Mat getUltrasoundImageCoordinate(int row, int col, double A, double B, double C, double D, short Ox, short Oy) {
	Mat coor = Mat::zeros(col, row, CV_16UC3);
	cout << "initial coordinate mat" << endl << coor.rows<<"*"<<coor.cols<<"*"<<coor.channels()<<":"<<coor.at<cv::Vec3s>(1,1) << endl;
	for (short i = 0; i < row; i++) {
		for (short j = 0; j < col; j++) {
			short z = (short)floor(-(A*j + B*j + D) / C);
			coor.at<cv::Vec3s>(i, j) = { i+Ox, j+Oy, z };
			//cout << coor.at<cv::Vec3s>(i, j);
		}
		//cout << endl;
		//cout << "finish " << i << " row!" << endl;
	}
	cout << "get ultrasound image coordinate is end!" << endl;
	return coor;
}

// 计算图像坐标
vector<Mat> getImageCoordinate(Mat worldcoor) {
	Mat leftcoor = Mat::zeros(worldcoor.rows, worldcoor.cols, CV_16UC2);
	Mat rightcoor = Mat::zeros(worldcoor.rows, worldcoor.cols, CV_16UC2);
	for (int i = 0; i < worldcoor.rows; i++) {
		for (int j = 0; j < worldcoor.cols; j++) {
			leftcoor.at<cv::Vec2s>(i, j)[0] = worldcoor.at<cv::Vec3s>(i, j)[0];
			leftcoor.at<cv::Vec2s>(i, j)[1] = worldcoor.at<cv::Vec3s>(i, j)[1];
			short z = worldcoor.at<cv::Vec3s>(i, j)[2];
			double parallax = z > 0 ? BASELINE*FOCUS*DX / z : 0;
			rightcoor.at<cv::Vec2s>(i, j)[0] = worldcoor.at<cv::Vec3s>(i, j)[0];
			rightcoor.at<cv::Vec2s>(i, j)[1] = worldcoor.at<cv::Vec3s>(i, j)[1] + (short)parallax+50;
			//cout << "z:" << z << ";parallax:" << (short)parallax << ";left:" << leftcoor.at<cv::Vec2s>(i, j)[0] << ";right:" << rightcoor.at<cv::Vec2s>(i, j)[0] << endl;
		}
	}
	// cout << "this is left coordinate:" << endl << leftcoor << endl << "this is right coordinate:" << endl << rightcoor << endl;
	vector<Mat> vm;
	vm.push_back(leftcoor);
	vm.push_back(rightcoor);
	cout << "this is vector<Mat>:" << endl << "left coordinate:" << endl << vm[0].rows<<"*"<<vm[0].cols << endl << "right coordinate:" << endl << vm[1].rows << "*" << vm[1].cols << endl;
	return vm;
}

// 图像融合
Mat imageFuse(Mat img, Mat ultsoundImg, Mat coor, float alpha) {
	int x, y;
	if (alpha > 1) { cout << "alpha must in [0,1]" << endl; exit(-1); }
	for (int i = 0; i < coor.rows; i++) {
		for (int j = 0; j < coor.cols; j++) {
			x = coor.at<Vec2s>(i, j)[0];
			y = coor.at<Vec2s>(i, j)[1];
			//cout << "[" << x << "," << y << "], ";
			img.at<Vec3b>(x, y) = alpha*img.at<Vec3b>(x, y) + (1 - alpha)*ultsoundImg.at<Vec3b>(i, j);
		}
	}
	cout << "image fuse is finished!!!" << endl;
	
	return img;
}

int main111() {
	cout << "main begining!!!" << endl;
	Mat img1 = Mat(Size(500, 500), CV_8UC3, Scalar::all(255));
	Mat ultrasoundImg = Mat(Size(4, 4), CV_8UC3, Scalar::all(40));
	resize(ultrasoundImg, ultrasoundImg, Size(20, 20));

	vector<Point2i> pointsPlex;
	pointsPlex.push_back(Point2i(60, 60));
	pointsPlex.push_back(Point2i(60, 69));
	pointsPlex.push_back(Point2i(60, 100));
	pointsPlex.push_back(Point2i(60, 109));
	pointsPlex.push_back(Point2i(50, 65));
	pointsPlex.push_back(Point2i(50, 70));
	cout << "points plex:" << endl << pointsPlex << endl;
	vector<Point3f> points3D = getProbe3DLocation(pointsPlex);
	//cout << "points 3d:" << endl << points3D << endl;
	Mat pointsMat = Mat(points3D).reshape(1,3).clone();
	cout << "points mat [" << pointsMat.rows << "*" << pointsMat.cols << "*" << pointsMat.channels() << "]:" << endl << pointsMat << endl;
	MatrixXf factors = getScanPanel(pointsMat);
	cout << "factors:" <<ONEPLEX<< endl << factors << endl;
	double sin_a = getSinAlpha(factors);
	cout << "sin_a:" << sin_a << endl;
	Mat imgz = getUltrasoundZ(Size(20, 20), 0.5);
	cout << "get ultrasound image z:" << endl << imgz << endl;
	Mat parallaxs = getUlltrasoundParallax(imgz, FOCUS, BASELINE);
	cout << "get ultrasound parallax mat:" << endl << parallaxs << endl;
	cout << "start point:" << pointsPlex[0] << endl;
	Mat fusedimg = imageFuse(img1, ultrasoundImg, parallaxs, pointsPlex[0], 0.5);
	imshow("fuse img", fusedimg);
	waitKey();
	system("pause");
	return 0;
}

int main11111111() {
	// 获得左右视频图像
	//Mat img1 = imread("C:/Users/Fyzer/Desktop/左.jpg");
	//Mat img2 = imread("C:/Users/Fyzer/Desktop/右.jpg");
	Mat img1 = Mat(Size(400, 400), CV_8UC3, Scalar::all(255));
	Mat img2 = Mat(Size(400, 400), CV_8UC3, Scalar::all(255));
	//Mat ultrasoundImg = imread("D:/document/ffmpeg/cs.png");
	// 获得超声图像
	Mat ultrasoundImg = Mat(Size(4, 4), CV_8UC3, Scalar::all(40));
	// 超声图像标准化
	resize(ultrasoundImg, ultrasoundImg, Size(40, 40));/*
	imshow("img", img);
	imshow("ultsound img", ultsoundImg);
	waitKey();*/
	// 三个坐标点--相机坐标系
	float kpa[3][3] = { 15,15,50, 15,20,50, 20,20,40 };
	//float kpa[3][4] = { 1,-3,0,1, 1,2,-1,1, 0,1,2,-2 };
	// 坐标点转化为矩阵
	Mat keyPoints = Mat(3, 3, CV_32FC1, kpa);
	// 得到平面方程的系数矩阵
	MatrixXf factors = getTargetPanel(keyPoints);
	// 得到超声图像坐标--相机坐标系
	Mat coor = getUltrasoundImageCoordinate(ultrasoundImg.rows, ultrasoundImg.cols, factors(0), factors(1), factors(2), factors(3), 15, 15);
	cout << "key points mat:" << endl << keyPoints << endl;
	cout << "this is the 3d panel's factors:" << endl << factors << endl;
	//cout << "this is ultrasound image coordinate:" << endl << coor << endl;
	//cout << "this is uiltrasound image coordinate:" << endl << coor.rows<<"*"<<coor.cols<<"*"<<coor.channels() << endl;
	vector<Mat> twomat = getImageCoordinate(coor);
	//cout << "this is ultrasound image left coor:" << endl << twomat[0] << endl << "this is ultrasound image right coor:" << endl << twomat[1] << endl;
	Mat leftfuse = imageFuse(img1, ultrasoundImg, twomat[0]);
	imshow("image fuse1", img1);
	waitKey();
	Mat rightfuse = imageFuse(img2, ultrasoundImg, twomat[1]);
	imshow("image fuse2", img2);
	waitKey();
	/*//cout << "left coordinate:" << endl << twomat[0] << endl;
	//cout << "right coordinate:" << endl << twomat[1] << endl;
	//cout << "coordinate divi:" << endl << twomat[0] - twomat[1] << endl;
	imshow("left fuse image", leftfuse);
	imshow("right fuse image", rightfuse);
	imwrite("C:/Users/Fyzer/Desktop/左f.jpg", leftfuse);
	imwrite("C:/Users/Fyzer/Desktop/右f.jpg", rightfuse);
	waitKey();/**/
	system("pause");
	return 0;
}