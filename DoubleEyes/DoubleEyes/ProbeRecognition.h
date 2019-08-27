#pragma once
#include "stdafx.h"


class ProbeRecognition {
public:
	static double angle;
	static vector<Point2f> probeRecognition(Mat img);
};