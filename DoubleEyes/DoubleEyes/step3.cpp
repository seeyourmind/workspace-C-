#include "stdafx.h"
#include "step1.h"
#include "step2.h"
#include <time.h>  // 用来产生随机数
#include <thread>  // 使用线程

void step1Task(Mat pic, string flag, vector<Point2f> vpf) {
	step1processing(pic, flag);
	return;
}

void step2Task(Mat frame, Mat cs, Mat mask, vector<Point2f> plexL, vector<Point2f> plexR) {
	step2processing(frame, cs, mask, plexL, plexR);
	return;
}

int main_fusionold() {
	// 读取叠加素材
	string root = "";
	Mat cs = imread(root + "siatcs_copy.png");
	Mat cs_sector = imread(root + "siatcs_sector_copy.png");
	cvtColor(cs_sector, cs_sector, CV_RGB2GRAY);
	resize(cs, cs, Size(cs.cols*5.5, cs.rows*8));
	resize(cs_sector, cs_sector, Size(cs_sector.cols*5.5, cs_sector.rows*8));
	//imshow("cs", cs);
	//waitKey(0);

	// 提取帧图像
	double t = (double)getTickCount();
	//*
	VideoCapture capture("D:\\document\\ffmpeg\\20190717095903.mp4");
	//VideoCapture capture("video3-20190609181048.avi");
	//capture.get十分强大 可以获得帧的很多信息
	int frame_width = (int)capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int frame_height = (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	int frame_number = capture.get(CV_CAP_PROP_FRAME_COUNT);

	//写视频
	VideoWriter out;
	out.open("copy_result_427450_right.avi", CV_FOURCC('X', 'V', 'I', 'D'), capture.get(CV_CAP_PROP_FPS), Size(frame_width/4, frame_height/2));

	srand((unsigned)time(NULL));
	long frameToStart = rand() % frame_number;
	frameToStart = 427;
	//cout << "帧开始的地方" << frameToStart << endl;
	int initidx = 0;
	vector<Point2f> coorp;
	coorp.push_back(Point2f(271.721, 224.58));
	coorp.push_back(Point2f(341.729, 375.212));
	coorp.push_back(Point2f(271.931, 230.682));
	coorp.push_back(Point2f(271.126, 241.449));
	coorp.push_back(Point2f(270.719, 238.13));
	coorp.push_back(Point2f(261.47, 250.965));
	coorp.push_back(Point2f(259.067, 267.426));
	coorp.push_back(Point2f(254.682, 274.802));
	coorp.push_back(Point2f(259.889, 292.604));
	coorp.push_back(Point2f(271.253, 224.404));
	coorp.push_back(Point2f(271.718, 223.445));
	coorp.push_back(Point2f(271.935, 215.818));
	coorp.push_back(Point2f(281.134, 211.094));
	coorp.push_back(Point2f(271.044, 197.421));
	coorp.push_back(Point2f(245.224, 194.388));
	coorp.push_back(Point2f(245.458, 203.016));
	coorp.push_back(Point2f(245.598, 194.949));
	coorp.push_back(Point2f(260.989, 140.832));
	coorp.push_back(Point2f(248.997, 173.999));
	coorp.push_back(Point2f(255.198, 164.348));

	while (frameToStart<=446 ) {
		capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
		Mat frame, frameL, frameR, roiL, roiR;
		if (!capture.read(frame)) {
			cout << "读取帧数据失败" << endl;
			return -1;
		}

		frameL = frame(Rect(0, 0, frame_width / 2, frame_height));
		frameR = frame(Rect(frame_width / 2, 0, frame_width / 2, frame_height));
		resize(frameL, frameL, Size(frameL.cols*0.5, frameL.rows*0.5));
		resize(frameR, frameR, Size(frameR.cols*0.5, frameR.rows*0.5));
		
		vector<Point2f> lvp = step1processing(frameR, "L");
		//cout << lvp << endl;
		vector<Point2f> plexL, plexR;
		plexL.push_back(Point2f(99.654816, 150.16751));
		plexL.push_back(Point2f(94.651764, 141.72487));
		plexL.push_back(Point2f(215.5076, 70.106598));
		plexR.push_back(Point2f(69.139099, 150.25458));
		plexR.push_back(Point2f(64.118835, 141.8511));
		plexR.push_back(Point2f(185.14618, 69.549042));
		//circle(frameL, lvp[0], 3, Scalar(0, 0, 255));
		Mat zm;
		if (lvp.size() == 0) {
			zm = frameR;
		}
		else {
			zm = step2processing(frameR, cs, cs_sector, plexL, plexR, lvp[1].x, coorp[initidx], 0.6, lvp[1].y);
		}
		imshow("result", zm);
		//out << zm;
		waitKey(10);
		//thread t1(step1Task, frameL, "L", *lvp);
		//t1.join();
		initidx += 1;
		frameToStart += 1;
	}
	capture.release();
	//out.release();
	/*capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
	Mat frame, frameL, frameR, roiL, roiR;
	if (!capture.read(frame)) {
		cout << "读取帧数据失败" << endl;
		return -1;
	}

	frameL = frame(Rect(0, 0, frame_width / 2, frame_height));
	frameR = frame(Rect(frame_width / 2, 0, frame_width / 2, frame_height));
	resize(frameL, frameL, Size(frameL.cols*0.5, frameL.rows*0.5));
	resize(frameR, frameR, Size(frameR.cols*0.5, frameR.rows*0.5));
	
	imshow("frame l", frameL);
	imshow("frame r", frameR);
	waitKey(10);
	//imwrite("frameL.jpg", frameL);
	//imwrite("frameR.jpg", frameR);
	capture.release();
	/*
	Mat frameL = imread("frameL500.jpg");
	Mat frameR = imread("frameL500.jpg");
	
	///double step1t = (double)getTickCount();
	vector<Point2f> lvp, rvp;
	thread t1(step1Task, frameL, "L", &lvp);
	t1.join(); */
	//thread t2(step1Task, frameR, "R", &rvp);
	//t2.join();
	///vector<Point2f> lvp = step1processing(frameL, "L");
	///vector<Point2f> rvp = step1processing(frameL, "R");
	///cout << "step1 finish!" << endl;
	///step1t -= (double)getTickCount();
	///printf("step1 execution time = %gms\n", abs(step1t)*1000. / getTickFrequency());
	/*
	// 读取叠加素材
	///double srct = (double)getTickCount();
	string root = "";
	Mat cs = imread(root + "siatcs_copy.png");
	Mat cs_sector = imread(root + "siatcs_sector_copy.png");
	cvtColor(cs_sector, cs_sector, CV_RGB2GRAY);
	resize(cs, cs, Size(180, 180));
	resize(cs_sector, cs_sector, Size(180, 180));
	///srct -= (double)getTickCount();
	///printf("load image execution time = %gms\n", abs(srct)*1000. / getTickFrequency());

	vector<Point2f> plexL, plexR;
	plexL.push_back(Point2f(99.654816, 150.16751));
	plexL.push_back(Point2f(94.651764, 141.72487));
	plexL.push_back(Point2f(215.5076, 70.106598));
	plexR.push_back(Point2f(69.139099, 150.25458));
	plexR.push_back(Point2f(64.118835, 141.8511));
	plexR.push_back(Point2f(185.14618, 69.549042));
	///double step2t = (double)getTickCount();
	thread t3(step2Task, frameL, cs, cs_sector, plexL, plexR);
	thread t4(step2Task, frameR, cs, cs_sector, plexL, plexR);
	t3.join(); t4.join();
	///step2t -= (double)getTickCount();
	///printf("step2 execution time =%gms\n", abs(step2t)*1000. / getTickFrequency());
	t -= (double)getTickCount();
	printf("execution time = %gms\n", abs(t)*1000. / getTickFrequency());
	//waitKey(0);
	*/
	system("pause");
	return 0;
}
/*
int main_new() {
	// 读取叠加素材
	double srct = (double)getTickCount();
	string root = "";
	Mat cs = imread(root + "siatcs_copy.png");
	Mat cs_sector = imread(root + "siatcs_sector_copy.png");
	cvtColor(cs_sector, cs_sector, CV_RGB2GRAY);
	resize(cs, cs, Size(180, 180));
	resize(cs_sector, cs_sector, Size(180, 180));
	srct -= (double)getTickCount();
	printf("load image execution time = %gms\n", abs(srct)*1000. / getTickFrequency());

	// 提取帧图像
	double t = (double)getTickCount();
	VideoCapture capture("D:\\document\\ffmpeg\\20190717095903.mp4");
	//VideoCapture capture("video3-20190609181048.avi");
	//capture.get十分强大 可以获得帧的很多信息
	int frame_width = (int)capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int frame_height = (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	int frame_number = capture.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "frame_number: " << frame_number << endl;
	Mat frame;
	///int i = 0;
	while (capture.isOpened()) {
		capture >> frame;
		if (frame.empty()) {
			cout << "finish!" << endl;
			break;
		}
		///i++;
		///printf("\r frame is %d", i);
		Mat frameL = frame(Rect(0, 0, frame_width / 2, frame_height));
		Mat frameR = frame(Rect(frame_width / 2, 0, frame_width / 2, frame_height));
		resize(frameL, frameL, Size(frameL.cols*0.5, frameL.rows*0.5));
		resize(frameR, frameR, Size(frameR.cols*0.5, frameR.rows*0.5));
		vector<Point2f> lvp, rvp;
		thread t1(step1Task, frameR, "R", &lvp);
		t1.join();
		//thread t2(step1Task, frameR, "R", &rvp);
		//t2.join();
		//waitKey(0);
		/*
		if (lvp.size() == 0 || rvp.size() == 0) {
			///cout << "本次for结束" << endl;
			continue;
		}
		else {
			///cout << "执行step2" << endl;
			vector<Point2f> plexL, plexR;
			plexL.push_back(Point2f(99.654816, 150.16751));
			plexL.push_back(Point2f(94.651764, 141.72487));
			plexL.push_back(Point2f(215.5076, 70.106598));
			plexR.push_back(Point2f(69.139099, 150.25458));
			plexR.push_back(Point2f(64.118835, 141.8511));
			plexR.push_back(Point2f(185.14618, 69.549042));
			double step2t = (double)getTickCount();
			thread t3(step2Task, frameL, cs, cs_sector, plexL, plexR);
			thread t4(step2Task, frameR, cs, cs_sector, plexL, plexR);
			t3.join(); t4.join();
		}
		//*
	}
	if (!capture.isOpened()) {
		cout << "open video error!" << endl;
		system("pause");
		return -1;
	}
	capture.release();

	t -= (double)getTickCount();
	printf("execution time = %gms\n", abs(t)*1000. / getTickFrequency());
	//waitKey(0);
	system("pause");
	return 0;
}
*/