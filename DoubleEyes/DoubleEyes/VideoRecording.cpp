#include "stdafx.h"
#include <thread>
#include <time.h>
#include <string>

#define VIDEO_SIZE Size(640,480)
#define OUTPUT_SIZE Size(640*2,480)
#define FRAME_RATE 15.0

void VideoRecordTask(int cameraIdx, int* flag) {
	VideoCapture capture(cameraIdx);
	if (capture.get(CV_CAP_PROP_HUE) != 0) {
		printf("摄像头%d断开，请检查线路是否连接正常！！！\n", cameraIdx);
		capture.release();
		return;
	}
	else {
		//cout << capture.get(CV_CAP_PROP_HUE) << endl;
		VideoWriter writer("video" + to_string(cameraIdx) + ".avi", CV_FOURCC('M', 'J', 'P', 'G'), FRAME_RATE, VIDEO_SIZE);
		while (1) {
			Mat frame;
			capture >> frame;
			writer << frame;
			if (frame.empty()) {
				cout << "Finish!!!" << endl;
				return;
			}
			imshow("frame" + cameraIdx, frame); 
			if (*flag == 27) return;
			*flag = waitKey(10);
		}
	}
}

int main_videorecord() {
	/*
	int flag = 0;
	thread t1(VideoRecordTask, 1, &flag);
	thread t2(VideoRecordTask, 2, &flag);
	t1.join(); 
	t2.join();
	*/
	VideoCapture capture1(1);
	VideoCapture capture2(2);
	time_t rawtime;
	struct tm ltm;
	time(&rawtime);
	localtime_s(&ltm, &rawtime);
	char buffer[16];
	sprintf_s(buffer, sizeof(buffer), "%4d%02d%02d%02d%02d%02d", (1900 + ltm.tm_year), (1 + ltm.tm_mon), ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
	string timestr(buffer);
	///cout << "local time is: " << timestr << endl;
	if (capture1.get(CV_CAP_PROP_HUE) != 0 || capture2.get(CV_CAP_PROP_HUE) != 0) {
		printf("摄像头断开，请检查线路是否连接正常！！！\n");
		capture1.release();
		return -1;
	}
	else {
		//cout << capture.get(CV_CAP_PROP_HUE) << endl;
		VideoWriter writer1("video1.avi", CV_FOURCC('M', 'J', 'P', 'G'), FRAME_RATE, VIDEO_SIZE);
		VideoWriter writer2("video2.avi", CV_FOURCC('M', 'J', 'P', 'G'), FRAME_RATE, VIDEO_SIZE);
		VideoWriter writer3("video3-" + timestr + ".avi", CV_FOURCC('M', 'J', 'P', 'G'), FRAME_RATE, OUTPUT_SIZE);
		///VideoWriter writer3("video3.avi", CV_FOURCC('M', 'J', 'P', 'G'), FRAME_RATE, OUTPUT_SIZE);
		double t = (double)getTickCount();
		printf("begin to make video >>>>>>>>>>>>>>");
		while (1) {
			Mat frame1, frame2, outFrame;
			capture1 >> frame1;
			capture2 >> frame2;
			hconcat(frame1, frame2, outFrame);
			writer1 << frame1;
			writer2 << frame2;
			writer3 << outFrame;
			if (frame1.empty() || frame2.empty()) {
				cout << "Finish!!!" << endl;
				break;
			}
			imshow("frame1", frame1);
			imshow("frame2", frame2);
			if (waitKey(10) == 27) {
				t -= (double)getTickCount();
				printf("make video time is %gs.\n", abs(t) / getTickFrequency());
				break;
			}
		}
	}
	///system("ffmpeg -i video1.avi -i video2.avi -filter_complex \"[0:v]pad=iw*2:ih[a];[a][1:v]overlay=w:0\" -y video_out.avi");
	system("pause");
	return 0;
}