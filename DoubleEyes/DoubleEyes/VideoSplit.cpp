#include "stdafx.h"

int main_videosplit() {
	// ��ȡ��Ƶ
	VideoCapture cap_up, cap_down;
	//cap_up.open("D:/document/about programs/SIAT/PPT/200px_angle_output_3/200px_angle_output_3_up.avi");
	//cap_down.open("D:/document/about programs/SIAT/PPT/200px_angle_output_3/200px_angle_output_3_down.avi");
	cap_up.open("copy_result_306328_left.avi");
	cap_down.open("copy_result_306328_right.avi");
	// ��ȡFPS
	double fps;
	fps = cap_up.get(CV_CAP_PROP_FPS);
	// ��ȡ��Ƶ�Ŀ��
	int framew, frameh;
	framew = cap_up.get(CV_CAP_PROP_FRAME_WIDTH);
	frameh = cap_down.get(CV_CAP_PROP_FRAME_HEIGHT);

	// ���ñ������ | ֻ֧��AVI��ʽ����Ƶ
	VideoWriter out;
	//out.open("D:/document/about programs/SIAT/PPT/200px_angle_output_3/200px_angle_output_3_LR.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps, Size(framew*2, frameh));
	out.open("fusion_result_306328.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps/4, Size(framew * 2, frameh));

	Mat frame_up, frame_down, frame_out;
	while(cap_up.isOpened() && cap_down.isOpened())
	{
		cap_up >> frame_up; //�ȼ���cap_up.read(frame);
		cap_down >> frame_down;
		if (frame_up.empty() && frame_down.empty())
		{
			cout << "cannot get the frame!!!" << endl;
			break;
		}
		hconcat(frame_up, frame_down, frame_out);
		out << frame_out;
	}
	cap_up.release();
	cap_down.release();
	out.release();
	cout << "Finish!!!" << endl;
	system("pause");
	return 0;
}