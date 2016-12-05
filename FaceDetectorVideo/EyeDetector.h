#pragma once

#include<opencv2\opencv.hpp>
#include<iostream>

using namespace cv;
using namespace std;

#define STATUS_MSG_SIZE (64)
#define WINDOW_NAME_ORIGIN_IMAGE "Origin Image"
#define WINDOW_NAME_MODIFY_IMAGE "Modify Image"
#define WINDOW_NAME_EYE "Eye"

enum EyeDetectorStatus {
	CAMERA_OPENED,
	CAPTURE_EYE,
	CAMERA_CLOSED,
	NO_CAMERA_AVAILABLE
};

enum RotFlag {
	ROT_0,
	ROT_CW_90,
	ROT_CCW_90,
	ROT_180
};

class EyeDetector {
public:
	EyeDetector();
	~EyeDetector();
	void init(VideoCapture&);
	void init(VideoCapture&, Size);
	void init(VideoCapture&, Size, RotFlag);

	bool update_image();

	void show_frame();

	bool detect_eye();

	void set_classifier(CascadeClassifier&, std::vector<Rect>&);

protected:

	void reportStatus(EyeDetectorStatus);
	template<size_t STR_SIZE>
	void reportStatus(EyeDetectorStatus, const char (&)[STR_SIZE]);

	void rot90(Mat&, RotFlag);

	bool blank_detect();

	VideoCapture* m_cap;
	Mat m_frame;
	Size cap_size;
	RotFlag cap_rot;
	CascadeClassifier* m_classifier;
	std::vector<Rect>* detected_object;

	unsigned proc_count;

};

