// DoubleEyes.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DoubleEyes.h"

int old()
{
	Mat img_src = imread("C:/Users/Fyzer/Desktop/GitHubUniverse.png");

	// 设定x_up和x_down
	int x_up = 60, x_down = x_up + (6 * 80 / 50);
	cout << x_up << ":" << x_down << endl;
	// 一些参数
	int y_up = 100, y_down = 100;
	// 标准化图像尺寸
	Mat img;
	resize(img_src, img, Size(100, 100));

	// 获取视频
	VideoCapture cap_up, cap_down;
	cap_up.open("D:/document/ffmpeg/Output/sm1_up.mp4");
	cap_down.open("D:/document/ffmpeg/Output/sm1_down.mp4");
	// 获取FPS
	double fps_up, fps_down;
	fps_up = cap_up.get(CV_CAP_PROP_FPS);
	fps_down = cap_down.get(CV_CAP_PROP_FPS);
	cout << "fps_up:" << fps_up << ";fps_down:" << fps_down << endl;
	// 获取视频的宽高
	Size size_up, size_down;
	size_up = Size(cap_up.get(CV_CAP_PROP_FRAME_WIDTH), cap_up.get(CV_CAP_PROP_FRAME_HEIGHT));
	size_down = Size(cap_down.get(CV_CAP_PROP_FRAME_WIDTH), cap_down.get(CV_CAP_PROP_FRAME_HEIGHT));
	// 设置保存规则 | 只支持AVI格式的视频
	VideoWriter out_up, out_down;
	out_up.open("D:/document/ffmpeg/Cout/output_up.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps_up, size_up);
	out_down.open("D:/document/ffmpeg/Cout/output_down.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps_down, size_down);
	// 逐帧融合
	Mat frame_up, frame_down;
	int timer = 0;
	while (cap_up.isOpened() && cap_down.isOpened())
	{
		cap_up >> frame_up; //等价于cap_up.read(frame);
		cap_down >> frame_down;
		if (frame_up.empty() && frame_down.empty())
		{
			cout << "cannot get the frame!!!" << endl;
			break;
		}
		mergeFrame(frame_up, img, x_up, 0.50);
		mergeFrame(frame_down, img, x_down, 0.50);

		out_up << frame_up;
		out_down << frame_down;
		timer++;
		system("cls");
		cout << "Making video...\n" << "Rest:" << cap_up.get(CV_CAP_PROP_FRAME_COUNT) - timer << endl;
	}
	cap_up.release();
	cap_down.release();
	out_up.release();
	out_down.release();
	destroyAllWindows();
	cout << "Finish!!!" << endl;
	system("pause");
    return 0;
}

int main_binocular_vision() {
	cout << "begin calculate" << endl;
	String imgPath = "D:/document/ffmpeg/cs.png";
	doubleEyesVideoFuse(srcVideoPath, outVideoPath, imgPath, false);
	return 0;
}

int main_deep() {
	Mat img1 = imread("D:/document/ffmpeg/sgbm/up.png");
	Mat img2 = imread("D:/document/ffmpeg/sgbm/down.png");
	Mat up = imread("D:/document/ffmpeg/sgbm/sgbm_up.png");
	Mat down = imread("D:/document/ffmpeg/sgbm/sgbm_down.png");
	getDeepImage(img1, img2);
	return 0;
}

// 双目视频处理
int doubleEyesVideoFuse(String srcVideoPath, String outVideoPath, String imgPath, bool vertical)
{
	Mat img_src = imread(imgPath);

	// 设定x_up和x_down
	int x_up = 60, x_down = x_up + (6 * 80 / 50);
	cout << x_up << ":" << x_down << endl;
	// 一些参数
	int y_up = 100, y_down = 100;
	// 标准化图像尺寸
	Mat img;
	resize(img_src, img, Size(100, 100));

	VideoCapture vcap;
	vcap.open(srcVideoPath + "/sm.mp4");// 获取视频源
	double fps = vcap.get(CV_CAP_PROP_FPS);// 获取帧率
	Size vcapSize;
	if(vertical) vcapSize = Size(vcap.get(CV_CAP_PROP_FRAME_WIDTH), vcap.get(CV_CAP_PROP_FRAME_HEIGHT));
	else vcapSize = Size(vcap.get(CV_CAP_PROP_FRAME_WIDTH)*2, vcap.get(CV_CAP_PROP_FRAME_HEIGHT)/2);// 获取宽高
	Size subvcapSize = Size(vcap.get(CV_CAP_PROP_FRAME_WIDTH), vcap.get(CV_CAP_PROP_FRAME_HEIGHT)/2);// 获取宽高
	// 保存视频
	VideoWriter out_up, out_down, out_;
	out_up.open(outVideoPath + "/c_output_up.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps, subvcapSize);
	out_down.open(outVideoPath + "/c_output_down.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps, subvcapSize);
	out_.open(outVideoPath + "/c_output_2.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps, vcapSize);
	cout << "vcap size:" << vcapSize << ";  fps:" << fps << endl;
	cout << "sub vcap size:" << subvcapSize << ";  fps:" << fps << endl;
	// 逐帧处理
	Mat frame, frame_up, frame_down;
	vector<Mat> outMats;
	int timer = 0;
	while (vcap.isOpened()) {
		vcap >> frame;
		if (frame.empty())
		{
			cout << "cannot get the frame!!!" << endl;
			break;
		}

		outMats = divideFrame(frame, 1);
		frame_up = outMats[0];
		frame_down = outMats[1];
		mergeFrameByAngle(frame_up, img, 0, 0.5);
		mergeFrameByAngle(frame_down, img, 30, 0.5);/*
		mergeFrame(frame_up, img, x_up, 0.50);/*
		imshow("up", frame_up);
		imwrite("up_fusion.jpg", frame_up);
		mergeFrame(frame_down, img, x_down, 0.50);/*
		imshow("down", frame_down);
		imwrite("down_fusion.jpg", frame_down);
		waitKey();*/

		Mat reframe = combineFrame(frame_up, frame_down, vertical);
		out_ << reframe;
		out_up << frame_up;
		out_down << frame_down;
		timer++;
		system("cls");
		cout << "Making video...\n" << "Rest:" << vcap.get(CV_CAP_PROP_FRAME_COUNT) - timer << endl;
	}
	vcap.release();
	out_up.release();
	out_down.release();
	destroyAllWindows();
	cout << "Finish!!!" << endl;
	system("pause");
	return 0;
}

// 逐像素叠加
Mat mergeFrame(Mat frame, Mat img, int bias, double percent) 
{
	if (img.empty()) 
	{
		cout << "please input the up image" << endl;
		exit(-1);
	}
	double percent_ = 1 - percent;
	for (int row = 0; row != img.rows; row++)
	{
		for (int col = 0; col != img.cols; col++)
		{
			frame.at<Vec3b>(row + bias, col + bias) = percent*img.at<Vec3b>(row, col) + percent_*frame.at<Vec3b>(row + bias, col + bias);
		}
	}

	return frame;
}
// 逐像素叠加
Mat mergeFrameByAngle(Mat frame, Mat img, int angle, double percent, int bias)
{
	if (img.empty())
	{
		cout << "please input the up image" << endl;
		exit(-1);
	}
	double percent_ = 1 - percent;
	Mat disparity = getDisparity(getDeep(Size(img.cols, img.rows), angle));/*
	ofstream outfile("data.txt", ios::app);
	outfile << "angle=" << angle << ":" << "\n" << disparity;
	outfile.close();*/
	for (int row = 0; row != img.rows; row++)
	{
		for (int col = 0; col != img.cols; col++)
		{
			int abias = (int)disparity.at<float>(row, col);
			//cout << "bias: " << bias << "; " << endl;
			frame.at<Vec3b>(bias + row + abias, bias + col + abias) = percent*img.at<Vec3b>(row, col) + percent_*frame.at<Vec3b>(bias + row + abias, bias + col + abias);
		}
	}
	
	return frame;
}

// 切割图像
vector<Mat> divideFrame(Mat &frame, bool vertical) 
{
	vector<Mat> outMats;
	int srcWidth, srcHight, subWidth, subHight;
	srcWidth = frame.cols;
	srcHight = frame.rows;
	if (vertical) {
		subWidth = srcWidth;
		subHight = srcHight / 2;
	}
	else {
		subWidth = int(srcWidth / 2);
		subHight = srcHight;
	}
	
	Mat sub1(subHight, subWidth, CV_8UC3, Scalar(0, 0, 0)); //row, col
	Mat sub2(subHight, subWidth, CV_8UC3, Scalar(0, 0, 0));

	Mat temp1, temp2;
	temp1 = frame(Rect(0, 0, sub1.cols, sub1.rows));//rect(x, y, width, height)选定感兴趣区域
	
	if (vertical) {
		temp2 = frame(Rect(0, subHight, sub2.cols, sub2.rows));
	}
	else {
		temp2 = frame(Rect(subWidth, 0, sub2.cols, sub2.rows));
	}
	//addWeighted(src, alpha, oth, beta, add, copy)//图像融合
	outMats.push_back(temp1);
	outMats.push_back(temp2);/*
	imshow("l", temp1);
	imshow("r", temp2);
	imwrite("up_src.jpg", temp1);
	imwrite("down_src.jpg", temp2);
	waitKey();*/
	return outMats;
}

// 图像拼接
Mat combineFrame(Mat up, Mat down, bool vertical) 
{
	Mat result;
	int upw, uph, downw, downh;
	upw = up.cols;
	uph = up.rows;
	downw = down.cols;
	downh = down.rows;
	if (vertical) {
		vconcat(up, down, result);
	}
	else {
		hconcat(up, down, result);
	}/*
	imshow("result", result);
	waitKey();*/
	return result;
}

// 图像位差图
Mat getDeepImage(Mat img1, Mat img2)
{
	namedWindow("left", 1);
	imshow("left", img1);
	namedWindow("right", 1);
	imshow("right", img2);
	waitKey();
	StereoSGBM sgbm;
	int SADWindowSize = 9;
	sgbm.preFilterCap = 63;
	sgbm.SADWindowSize = SADWindowSize > 0 ? SADWindowSize : 3;
	int cn = img1.channels();
	int numberOfDisparities = 64;
	sgbm.P1 = 8 * cn*sgbm.SADWindowSize*sgbm.SADWindowSize;
	sgbm.P2 = 32 * cn*sgbm.SADWindowSize*sgbm.SADWindowSize;
	sgbm.minDisparity = 0;
	sgbm.numberOfDisparities = numberOfDisparities;
	sgbm.uniquenessRatio = 10;
	sgbm.speckleWindowSize = 100;
	sgbm.speckleRange = 32;
	sgbm.disp12MaxDiff = 1;
	Mat disp, disp8;
	int64 t = getTickCount();
	sgbm((Mat)img1, (Mat)img2, disp);
	t = getTickCount() - t;
	cout << "Time elapsed:" << t * 1000 / getTickFrequency() << endl;
	disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities*16.));

	namedWindow("disparity", 1);
	imshow("disparity", disp8);
	waitKey();
	cvDestroyAllWindows();
	return disp8;
}

// 计算深度
Mat getDeep(Size imgSise, int angle, double block_height) {
	int row = imgSise.height;
	int col = imgSise.width;
	double radian = (angle*PI) / 180;
	Mat deepMat = Mat::zeros(row, col, CV_32FC1);
	for (int i = 0; i < row; i++)
	{
		Mat temp(1, col, CV_32FC1, Scalar((i + 1)*block_height*sin(radian)));
		temp.copyTo(deepMat.row(i));
	}
	// cout << deepMat << endl;
	return deepMat;
}

// 计算视差
Mat getDisparity(Mat deep, double focus, double baseline) {
	Mat disparity = Mat::zeros(deep.rows, deep.cols, CV_32FC1);
	disparity = focus*baseline / deep;
	//cout << disparity << endl;
	return disparity;
}
