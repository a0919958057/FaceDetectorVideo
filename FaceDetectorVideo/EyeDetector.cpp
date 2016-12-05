#include "EyeDetector.h"
#include <stdlib.h>
#include <stdio.h>


EyeDetector::EyeDetector() :
	cap_size(640, 480),
	m_cap(nullptr),
	cap_rot(RotFlag::ROT_0),
	m_classifier(nullptr),
	proc_count(0) {

	// Setup the Window property
	namedWindow(WINDOW_NAME_ORIGIN_IMAGE, WINDOW_AUTOSIZE);
	namedWindow(WINDOW_NAME_MODIFY_IMAGE, WINDOW_AUTOSIZE);
	namedWindow(WINDOW_NAME_EYE, WINDOW_AUTOSIZE);

	// Relocation the Window
	moveWindow(WINDOW_NAME_ORIGIN_IMAGE, 10, 10);
	moveWindow(WINDOW_NAME_MODIFY_IMAGE, 360+10, 10);
	moveWindow(WINDOW_NAME_EYE, 360 + 10 + 360 + 10, 10);
}


EyeDetector::~EyeDetector() {
	
}

void EyeDetector::init(VideoCapture &cap) {
	m_cap = &cap;

	if (!m_cap->isOpened()) {

		reportStatus(EyeDetectorStatus::NO_CAMERA_AVAILABLE, "Not open");

	} else {
		char cap_info[STATUS_MSG_SIZE] = { 0 };
		sprintf_s(cap_info, "Width: %f,  Height: %f",
			cap.get(CAP_PROP_FRAME_WIDTH),
			cap.get(CAP_PROP_FRAME_HEIGHT));

		reportStatus(EyeDetectorStatus::CAMERA_OPENED, cap_info);
	}

	return;
}

void EyeDetector::init(VideoCapture& cap, Size frameSize) {
	cap_size = frameSize;
	init(cap);
}

void EyeDetector::init(VideoCapture& cap, Size frameSize, RotFlag rotate) {
	cap_size = frameSize;
	cap_rot = rotate;
	init(cap);
}

bool EyeDetector::update_image() {

	if (!m_cap->isOpened()) {
		reportStatus(EyeDetectorStatus::NO_CAMERA_AVAILABLE, "Can't get a frame");
		return false;
	}
	*m_cap >> m_frame;
	resize(m_frame, m_frame, cap_size, 0.0, 0.0, CV_INTER_LINEAR);
	//Mat rot_mat = getRotationMatrix2D(Point2f(cap_size / 2), cap_rotate, 1.0);
	//transpose(frame, frame);
	//flip(frame, frame,1);
	//warpAffine(frame, frame, rot_mat, cap_size);
	rot90(m_frame, cap_rot);
	proc_count++;
	return true;
}

void EyeDetector::show_frame() {
	imshow(WINDOW_NAME_ORIGIN_IMAGE, m_frame);
	Mat modify_frame;
	modify_frame = m_frame.clone();

	if (m_classifier != nullptr) {
		
		bool is_eye_detected = detect_eye();
		for each (auto eyes in (*detected_object)) {
			if (is_eye_detected)
				rectangle(modify_frame, eyes, CV_RGB(255, 0, 0), 2);
			else {
				rectangle(modify_frame, eyes, CV_RGB(0, 0, 255), 2);
			}

			/****** Add the text on image ******/
			char str_buffer[20] = { 0 };
			sprintf_s(str_buffer, "(%2.2f,%2.2f)", 
				(double)cap_size.height/ (double)eyes.width, 
				(double)cap_size.width/ (double)eyes.height);
			putText(modify_frame, String(str_buffer), Point(eyes.x, eyes.y - 5), FONT_HERSHEY_COMPLEX, 0.5, CV_RGB(255,255,255), 1.0,8);
			sprintf_s(str_buffer, "%fFPS",m_cap->get(CAP_PROP_FPS));
			putText(modify_frame, String(str_buffer), Point(0, 20), FONT_HERSHEY_SIMPLEX, 0.7, CV_RGB(255, 255, 255), 1.5);
			sprintf_s(str_buffer, "Proc Count %d", proc_count);
			putText(modify_frame, String(str_buffer), Point(0, 40), FONT_HERSHEY_SIMPLEX, 0.7, CV_RGB(255, 255, 255), 1.5);
			/***********************************/
			Mat eye_roi(m_frame, eyes);
			Mat hsv_eye;
			cvtColor(eye_roi, hsv_eye, CV_RGB2HSV);
			vector<Mat> hsv_planes;
			split(hsv_eye, hsv_planes);
			Mat v_hist;
			
			//equalizeHist(hsv_planes[2], v_hist);
			threshold(hsv_planes[2], v_hist, 30, 255, ThresholdTypes::THRESH_BINARY);
			imshow("Equalized Image", v_hist);

			imshow(WINDOW_NAME_EYE, hsv_planes[2]);

		}
		imshow(WINDOW_NAME_MODIFY_IMAGE, modify_frame);
	}
}

bool EyeDetector::detect_eye() {
	vector<Rect> eye_new;

	m_classifier->detectMultiScale(
		m_frame,
		eye_new,
		1.1,
		5,
		CV_HAAR_MAGIC_VAL,
		Size(cap_size.height / 5, cap_size.width / 10),
		Size(cap_size.height / 2, cap_size.width / 7));
	if (eye_new.size() > 0) {
		*detected_object = eye_new;
		return true;
	}
	else {
		return false;
	}
}

void EyeDetector::set_classifier(CascadeClassifier &classifier, std::vector<Rect> &detected_set) {
	m_classifier = &classifier;
	detected_object = &detected_set;
}

void EyeDetector::reportStatus(EyeDetectorStatus status) {
	return reportStatus(status, "NULL");
}

void EyeDetector::rot90(Mat &image, RotFlag flag) {
	switch (flag) {
		case RotFlag::ROT_CW_90 :
			transpose(image, image);
			flip(image, image, 1);
			break;
		case RotFlag::ROT_CCW_90 :
			transpose(image, image);
			flip(image, image, 0);
			break;
		case RotFlag::ROT_180 :
			flip(image, image, -1);
		default:
			break;
	}
}

bool EyeDetector::blank_detect() {

	return false;
}

template<size_t STR_SIZE>
void EyeDetector::reportStatus(EyeDetectorStatus status, const char (&metaMsg)[STR_SIZE]) {
	
	// Reset string variable
	char statusMsg[STATUS_MSG_SIZE] = { 0 };

	std::stringstream stream;

	stream.rdbuf()->pubsetbuf(statusMsg, sizeof(statusMsg));

	stream << "Eye Detector [Status]:\t";

	switch (status) {
		case EyeDetectorStatus::CAMERA_CLOSED:
			stream << "Camera was closed";
			break;
		case EyeDetectorStatus::CAMERA_OPENED:
			stream << "Camera was opened";
			break;
		case EyeDetectorStatus::CAPTURE_EYE:
			stream << "Eyes Capture";
			break;
		case EyeDetectorStatus::NO_CAMERA_AVAILABLE:
			stream << "No camera available";
			break;
		default:
			break;
	}
	stream << "\t <" << metaMsg << ">" ;


	string str;
	getline(stream, str);
	cout << str << endl;
	
	return;
}
