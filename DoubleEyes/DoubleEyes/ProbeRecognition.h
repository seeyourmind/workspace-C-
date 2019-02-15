#pragma once
#include "stdafx.h"
#include <ostream>
#include <opencv2\opencv.hpp> //imread

using namespace std;
using namespace cv;

class ProbeRecognition {
public:
	static vector<Point2f> probeRecognition(Mat img);
private:
	Mat getProbeMarkUseOTSU(Mat img);
	Mat getProbeMarkUseKmeans(Mat pic);
	Mat morphologicalTreatment(Mat img, int type, Size size);
	vector<Point2f> getProbeMarkUseContours(Mat src);
};