#include "stdafx.h"
#include "UltrasoundImageFusion.h"

double onePlexForCamera = 1.0;//相机坐标系中单位像素
int UltrasoundImageFusion::maxvalue_xl = 0;
int UltrasoundImageFusion::maxvalue_xr = 0;
int UltrasoundImageFusion::maxvalue_y = 0;

// 关键点像素坐标转相机坐标
vector<vector<Point3f>> keyPointsFromPlex2Camera(vector<Point2f> leftImg, vector<Point2f> rightImg, Size imgSize) {
	vector<vector<Point3f>> cameraCoors;
	vector<Point3f> leftCameraCoor, rightCameraCoor;
	int U0 = imgSize.width / 2;
	int V0 = imgSize.height / 2;
	for (int i = 0; i < leftImg.size(); ++i) {
		float delta = ONEPLEX*(leftImg[i].x - rightImg[i].x);
		leftCameraCoor.push_back(Point3f(ONEPLEX*BASELINE*(leftImg[i].x - U0) / delta, ONEPLEX*BASELINE*(leftImg[i].y - V0) / delta, BASELINE*FOCUS / delta));
		rightCameraCoor.push_back(Point3f(ONEPLEX*BASELINE*(rightImg[i].x - U0) / delta, ONEPLEX*BASELINE*(rightImg[i].y - V0) / delta, BASELINE*FOCUS / delta));
	}
	onePlexForCamera = (leftCameraCoor[0].x - leftCameraCoor[0].y) / (leftImg[0].x - leftImg[0].y);
	cameraCoors.push_back(leftCameraCoor);
	cameraCoors.push_back(rightCameraCoor);
	return cameraCoors;
}

// 求平面方程的四个系数
MatrixXf getScanPanelFactors(Mat keyPoints) {
	return getFactor(keyPoints);
}

// 计算平面上的各点坐标，并将其转换为像素坐标
vector<Mat> getEachCoorFromCamera2Plex(double factors0, double factors1, double factors2, double factors3, Size imgSize) {
	//ofstream fout("mattxt.txt");
	vector<Mat> plexcoor;
	UltrasoundImageFusion uif;
	Mat ulcoor = Mat::zeros(imgSize, CV_16SC1);
	Mat urcoor = Mat::zeros(imgSize, CV_16SC1);
	Mat vcoor = Mat::zeros(imgSize, CV_16SC1);

	for (int i = 0; i < imgSize.width; ++i) {
		short* ulp = ulcoor.ptr<short>(i);
		short* urp = urcoor.ptr<short>(i);
		short* vp = vcoor.ptr<short>(i);
		for (int j = 0; j < imgSize.height; ++j) {
			float x = onePlexForCamera*i;
			float y = onePlexForCamera*j;
			float z = -(factors0*x + factors1*y + factors3) / factors2;

			int ul = FOCUS*x / z;
			int ur = FOCUS*(x - BASELINE) / z;
			int v = FOCUS*y / z;

			urp[j] = abs(ur);
			vp[j] = abs(v);
			ulp[j] = abs(ul);

			if (uif.maxvalue_xl < ulp[j]) uif.maxvalue_xl = ulp[j];
			if (uif.maxvalue_xr < urp[j]) uif.maxvalue_xr = urp[j];
			if (uif.maxvalue_y < vp[j]) uif.maxvalue_y = vp[j];
		}
	}
	plexcoor.push_back(ulcoor);//left col
	plexcoor.push_back(urcoor);//right col
	plexcoor.push_back(vcoor);//row

	return plexcoor;
}

// 图像叠加
Mat getUltrasoundFuseImage(Mat backgroud, Mat ultrasound, vector<Mat> coorMat, Point2f start, double angle, float alpha) {
	//double t = (double)getTickCount();
	Mat ucoor = coorMat[0];
	Mat vcoor = coorMat[1];
	for (int i = 0; i < ucoor.rows; ++i) {
		short* up = ucoor.ptr<short>(i);
		short* vp = vcoor.ptr<short>(i);
		Vec3b* ultrap = ultrasound.ptr<Vec3b>(i);
		Vec3b* srcp;
		for (int j = 0; j < ucoor.cols; ++j) {
			//double x = (double)up[j];
			//double y = (double)vp[j];
			//int u = start.y + x * cos(angle) - y * sin(angle);
			//int v = start.x + x * sin(angle) + y * cos(angle);
			double tt = (double)getTickCount();
			int u = start.y + (double)up[j] * cos(angle) - (double)vp[j] * sin(angle);
			int v = start.x + (double)up[j] * sin(angle) + (double)vp[j] * cos(angle);
			srcp = backgroud.ptr<Vec3b>(u);
			Vec3b temp = alpha*srcp[v] + (1 - alpha)*ultrap[j];
			tt -= (double)getTickCount();
			//printf("calculate u/v and temp execution time = %gms.\n", abs(tt)*1000. / getTickFrequency());

			srcp[v] = temp;
			srcp[v - 1] = temp;
			srcp[v + 1] = temp;
			srcp = srcp - 1;///////////////
			srcp[v] = temp;
			srcp[v - 1] = temp;
			srcp[v + 1] = temp;
			srcp = srcp + 1;////////////////
			srcp[v] = temp;
			srcp[v - 1] = temp;
			srcp[v + 1] = temp;
			/*double tt = (double)getTickCount();
			tt -= (double)getTickCount();
			printf("assignment execution time = %gms.\n", abs(tt)*1000. / getTickFrequency());*/
		}
	}
	//t -= (double)getTickCount();
	//printf("getUltrasoundImage execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	return backgroud;
}

void getUltrasoundFuseImageTask(Mat &background, Mat ultrasoundImg, vector<Mat> plexcoors, Point2f Ps, double angle, float alpha) {
	background = getUltrasoundFuseImage(background, ultrasoundImg, plexcoors, Ps, angle, alpha);
	return;
}

void UltrasoundImageFusion::ultrasoundImageFusion(Mat backgroundL, Mat backgroundR, Mat ultrasoundImg, vector<Point2f> leftPs, vector<Point2f> rightPs, double angle, double alpha) {
	//图像尺寸调整
	UltrasoundImageFusion uif;
	Size backgroudResize = Size(400, 400);
	Size ultrasoundImgResize = Size(200, 200);
	/*resize(backgroundL, backgroundL, backgroudResize);
	resize(backgroundR, backgroundR, backgroudResize);
	resize(ultrasoundImg, ultrasoundImg, ultrasoundImgResize);*/
	
	//坐标运算
	vector<vector<Point3f>> key3d = keyPointsFromPlex2Camera(leftPs, rightPs, backgroudResize);
	MatrixXf factorsL = getScanPanelFactors(Mat(key3d[0]).reshape(1, 3).clone());
	vector<Mat> plexcoors = getEachCoorFromCamera2Plex(factorsL(0), factorsL(1), factorsL(2), factorsL(3), ultrasoundImgResize);
	//根据结果调整图像尺寸
	double maxx2, maxy2;
	maxx2 = uif.maxvalue_xl;
	maxy2 = uif.maxvalue_y;
	int maxxxx = uif.maxvalue_xl > uif.maxvalue_y ? uif.maxvalue_xl : uif.maxvalue_y;
	if (maxxxx>250) resize(backgroundL, backgroundL, Size(maxxxx * 2, maxxxx * 2));/*
	Mat t;
	plexcoors[0].convertTo(t, CV_16SC1);
	minMaxLoc(t, NULL, &minx, NULL, NULL);
	plexcoors[1].convertTo(t, CV_16SC1);
	minMaxLoc(t, NULL, &miny, NULL, NULL);
	cout << "CV_16SC1: max x = " << maxx << "; max y = " << maxy << endl;*/

	//融合
	angle = -angle*PI / 180.;
	vector<Mat> plexcoorsL(plexcoors);
	plexcoorsL.erase(plexcoorsL.begin() + 1);
	vector<Mat> plexcoorsR(plexcoors);
	plexcoorsR.erase(plexcoorsR.begin());
	double t = (double)getTickCount();
	thread t1(getUltrasoundFuseImageTask, backgroundL, ultrasoundImg, plexcoorsL, leftPs[0], angle, alpha);
	thread t2(getUltrasoundFuseImageTask, backgroundR, ultrasoundImg, plexcoorsR, rightPs[0], angle, alpha);
	t1.join();t2.join();
	t -= (double)getTickCount();
	printf("thread getUltrasoundFuseImage execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	//Mat remat = getUltrasoundFuseImage(backgroundL, ultrasoundImg, plexcoorsL, leftPs[0], -angle, alpha);
	//remat = getUltrasoundFuseImage(backgroundR, ultrasoundImg, plexcoorsR, rightPs[0], angle, alpha);
	imshow("remat", backgroundL);
	waitKey(1 );
}
/*
void UltrasoundImageFusion::ultrasoundImageFusion(Mat background, Mat ultrasoundImg, vector<Point2f> leftPs, vector<Point2f> rightPs, Point2f startPoint, double alpha) {
	//图像尺寸调整
	double t = (double)getTickCount();
	UltrasoundImageFusion uif;
	Size backgroudResize = Size(400, 400);
	Size ultrasoundImgResize = Size(250, 250);
	resize(background, background, backgroudResize);
	resize(ultrasoundImg, ultrasoundImg, ultrasoundImgResize);
	t -= (double)getTickCount();
	printf("resize execution time = %gms.\n", abs(t)*1000. / getTickFrequency());

	//坐标运算
	t = (double)getTickCount();
	vector<vector<Point3f>> key3d = uif.keyPointsFromPlex2Camera(leftPs, rightPs, backgroudResize);
	t -= (double)getTickCount();
	printf("keyPointsFromPlex2Camera execution time = %gms.\n", abs(t)*1000. / getTickCount());
	//cout << "the 3d points are:" << key3d[0] << ";" << key3d[1] << endl;
	t = (double)getTickCount();
	MatrixXf factorsL = uif.getScanPanelFactors(Mat(key3d[0]).reshape(1, 3).clone());
	t -= (double)getTickCount();
	printf("getScanPanelFactors execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	t = (double)getTickCount();
	vector<Mat> plexcoors = uif.getEachCoorFromCamera2Plex(factorsL, ultrasoundImgResize);
	t -= (double)getTickCount();
	printf("getEachCoorFromCamera2Plex execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	//cout << "left coordinate get panel factor:" << endl << factorsL << endl;
	//cout << "get plex coordinate of each point on panel:" << endl << plexcoors[0] << endl << plexcoors[1] << endl;
	//根据结果调整图像尺寸
	t = (double)getTickCount();
	double maxx, maxy;
	minMaxLoc(plexcoors[0], NULL, &maxx, NULL, NULL);
	minMaxLoc(plexcoors[1], NULL, &maxy, NULL, NULL);
	//cout << "max x:" << maxx << "  ;max y:" << maxy << endl;
	int maxxxx = maxx > maxy ? maxx : maxy;
	if (maxxxx>200) resize(background, background, Size(maxxxx * 2, maxxxx * 2));
	t -= (double)getTickCount();
	printf("minMaxLoc execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	//融合
	t = (double)getTickCount();
	plexcoors.erase(plexcoors.begin() + 1);
	Mat remat = uif.getUltrasoundFuseImage(background, ultrasoundImg, plexcoors, startPoint, 0, alpha);
	t -= (double)getTickCount();
	printf("getUltrasoundFuseImage execution time = %gms.\n", abs(t)*1000. / getTickFrequency());
	imshow("remat", remat);
	int rn = rand() % 100;
	//cout << "random number is:" << rn << endl;
	imwrite(to_string(rn)+"remat.jpg", remat);
	waitKey(1000);
}*/