#include "stdafx.h"
#include "UltrasoundImageFusion.h"

void UltrasoundImageFusion::ultrasoundImageFusion(Mat background, Mat ultrasoundImg, vector<Point2f> leftPs, vector<Point2f> rightPs, Point2f startPoint, double alpha) {
	//图像尺寸调整
	UltrasoundImageFusion uif;
	Size backgroudResize = Size(400, 400);
	Size ultrasoundImgResize = Size(400, 400);
	resize(background, background, backgroudResize);
	resize(ultrasoundImg, ultrasoundImg, ultrasoundImgResize);
	//坐标运算
	vector<vector<Point3f>> key3d = uif.keyPointsFromPlex2Camera(leftPs, rightPs, backgroudResize);
	MatrixXf factorsL = uif.getScanPanelFactors(Mat(key3d[0]).reshape(1, 3).clone());
	vector<Mat> plexcoors = uif.getEachCoorFromCamera2Plex(factorsL, ultrasoundImgResize);
	//cout << "left coordinate get panel factor:" << endl << factorsL << endl;
	//cout << "get plex coordinate of each point on panel:" << endl << plexcoors[0] << endl << plexcoors[1] << endl;
	//根据结果调整图像尺寸
	double maxx, maxy, minx, miny;
	minMaxLoc(plexcoors[0], &minx, &maxx);
	minMaxLoc(plexcoors[1], &miny, &maxy);
	//cout << "max x:" << maxx << "  ;max y:" << maxy << endl;
	int maxxxx = maxx > maxy ? maxx : maxy;
	if (maxxxx>200) resize(background, background, Size(maxxxx * 2, maxxxx * 2));
	//融合
	Mat remat = uif.getUltrasoundFuseImage(background, ultrasoundImg, plexcoors, startPoint, alpha);
	imshow("remat", remat);
	int rn = rand() % 100;
	cout << "random number is:" << rn << endl;
	imwrite(to_string(rn)+"remat.jpg", remat);/*
	waitKey(1000);*/
}

// 关键点像素坐标转相机坐标
vector<vector<Point3f>> UltrasoundImageFusion::keyPointsFromPlex2Camera(vector<Point2f> leftImg, vector<Point2f> rightImg, Size imgSize) {
	vector<vector<Point3f>> cameraCoors;
	vector<Point3f> leftCameraCoor, rightCameraCoor;
	int U0 = imgSize.width / 2;
	int V0 = imgSize.height / 2;
	for (int i = 0; i < leftImg.size(); i++) {
		float delta = DX*(leftImg[i].x - rightImg[i].x);
		leftCameraCoor.push_back(Point3f(DX*BASELINE*(leftImg[i].x - U0) / delta, DY*BASELINE*(leftImg[i].y - V0) / delta, BASELINE*FOCUS / delta));
		rightCameraCoor.push_back(Point3f(DX*BASELINE*(rightImg[i].x - U0) / delta, DY*BASELINE*(rightImg[i].y - V0) / delta, BASELINE*FOCUS / delta));
	}
	cameraCoors.push_back(leftCameraCoor);
	cameraCoors.push_back(rightCameraCoor);
	return cameraCoors;
}

// 求平面方程的四个系数
MatrixXf UltrasoundImageFusion::getScanPanelFactors(Mat keyPoints) {
	return getFactor(keyPoints);
}

// 计算平面上的各点坐标，并将其转换为像素坐标
vector<Mat> UltrasoundImageFusion::getEachCoorFromCamera2Plex(MatrixXf factors, Size imgSize) {
	ofstream fout("mattxt.txt");
	vector<Mat> plexcoor;
	Mat ucoor = Mat::zeros(imgSize, CV_32SC1);
	Mat vcoor = Mat::zeros(imgSize, CV_32SC1);
	for (int i = 0; i < imgSize.width; i++) {
		for (int j = 0; j < imgSize.height; j++) {
			float x = ONEPLEX*i;
			float y = ONEPLEX*j;
			float z = -(factors(0)*DX*i + factors(1)*DY*j + factors(3)) / factors(2);
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
Mat UltrasoundImageFusion::getUltrasoundFuseImage(Mat backgroud, Mat ultrasound, vector<Mat> coorMat, Point2f start, float alpha) {
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
	Mat background = Mat(Size(400, 400), CV_8UC3, Scalar::all(255));
	Mat ultrasoundImg = imread("D:/document/ffmpeg/cs.png");
	vector<Point2f> leftPs, rightPs;
	leftPs.push_back(Point2f(60, 60));
	rightPs.push_back(Point2f(69, 60));
	leftPs.push_back(Point2f(100, 60));
	rightPs.push_back(Point2f(109, 60));
	leftPs.push_back(Point2f(60, 50));
	rightPs.push_back(Point2f(69, 50));
	double alpha = 0.2;
	UltrasoundImageFusion::ultrasoundImageFusion(background, ultrasoundImg, leftPs, rightPs, leftPs[0], alpha);
	return 0;
}*/