#include "EyeDetector.h"
#include <stdlib.h>
#include <stdio.h>

EyeDetector* EyeDetector::self;

EyeDetector::EyeDetector() :
	cap_size(640, 480),
	m_cap(nullptr),
	cap_rot(RotFlag::ROT_0),
	m_classifier(nullptr),
	proc_count(0),
	is_datafile_open(false) {

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
	if (app_blink_data.is_open()) app_blink_data.close();
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

	self = this;
	setMouseCallback(WINDOW_NAME_EYE, onMouse, 0);

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
	bool isDetected_blink(false);

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

			// Register the eye frame
			m_eye = Mat(m_frame, eyes);
			// Register the eye search frame
			Rect eyes_sch_win(eyes);
			eyes.width += 50;
			eyes.height += 50;
			eyes.x -= 25;
			eyes.y -= 25;
			m_eye_window = Mat(m_frame, eyes);
			cvtColor(m_eye_window, m_eye_window, CV_RGB2GRAY);

			Mat hsv_eye;
			cvtColor(m_eye, hsv_eye, CV_RGB2HSV);
			cvtColor(m_eye, m_eye, CV_RGB2GRAY);
			//vector<Mat> hsv_planes;
			//split(hsv_eye, hsv_planes);
			//Mat v_hist;

			////equalizeHist(hsv_planes[2], v_hist);
			//threshold(hsv_planes[2], v_hist, 30, 255, ThresholdTypes::THRESH_BINARY);
			//imshow("Equalized Image", v_hist);

			/********* Detect Eye *********/
			isDetected_blink = detect_blink();
			/******************************/

			imshow(WINDOW_NAME_EYE, m_eye);

			/****** Add the text on image ******/
			char str_buffer[64] = { 0 };
			sprintf_s(str_buffer, "(%2.2f,%2.2f)", 
				(double)cap_size.height/ (double)eyes.width, 
				(double)cap_size.width/ (double)eyes.height);
			putText(modify_frame, String(str_buffer), Point(eyes.x, eyes.y - 5), FONT_HERSHEY_COMPLEX, 0.5, CV_RGB(255,255,255), 1.0,8);
			sprintf_s(str_buffer, "%fFPS",m_cap->get(CAP_PROP_FPS));
			putText(modify_frame, String(str_buffer), Point(0, 20), FONT_HERSHEY_SIMPLEX, 0.7, CV_RGB(255, 255, 255), 1.5);
			sprintf_s(str_buffer, "Proc Count %d", proc_count);
			putText(modify_frame, String(str_buffer), Point(0, 40), FONT_HERSHEY_SIMPLEX, 0.7, CV_RGB(255, 255, 255), 1.5);
			sprintf_s(str_buffer, "Compare value = %f", minVal);
			putText(modify_frame, String(str_buffer), Point(0, 60), FONT_HERSHEY_SIMPLEX, 0.7, CV_RGB(255, 255, 255), 1.5);
			/***********************************/

			
			

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

bool EyeDetector::detect_blink() {
	static int image_count(0);
	stringstream ss;
	ss << ".//template//temp" << image_count << ".jpg";
	string str;
	ss >> str;

	Mat image_template = imread(str.c_str(),CV_8U);


	// Using Template match
	Mat result;
	//blur(m_eye_window, m_eye_window, Size(10, 10));
	//blur(image_template, image_template, Size(10, 10));

	
	matchTemplate(m_eye_window, image_template, result, CV_TM_SQDIFF_NORMED);
	minMaxLoc(result, &minVal, 0, &minLoc, 0);

	// Using Histgram Compare
	MatND hist_base;
	MatND hist_test;
	MatND error;

	/// Using 128 bins for gray value
	int g_bins = 128;
	int histSize[] = { g_bins };

	float s_ranges[] = { 0, 256 };
	const float* range[] = { s_ranges };

	int channels[] = { 0 };
	calcHist(&image_template, 1, channels, Mat(), hist_base , 1, histSize, range);
	normalize(hist_base, hist_base, 0, 1, NORM_MINMAX, -1, Mat());

	calcHist(&m_eye, 1, channels, Mat(), hist_test, 1, histSize, range);
	normalize(hist_test, hist_test, 0, 1, NORM_MINMAX, -1, Mat());

	absdiff(hist_base, hist_test, error);

	//double result = compareHist(image_template, m_eye, HistCompMethods::HISTCMP_CORREL);
	Scalar sumError = sum(error);
	absSum = sumError[0];

	if (minVal > 4744717) return true;

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


void EyeDetector::onMouse(int aEvent, int x, int y, int flags, void* param) { 

	self->doMemberMouseCallback(aEvent, x, y, flags);
	return;
}

void EyeDetector::doMemberMouseCallback(int aEvent, int x, int y, int flags) {
	static int image_count(0);
	if (aEvent != EVENT_LBUTTONDOWN)
		return;

	save_eyeimage(image_count++);
}


bool EyeDetector::record_data(int data_id) {
	if (!is_datafile_open) {
		is_datafile_open = open_recordfile(data_id);
		return false;
	}

	// Get past time
	QueryPerformanceCounter(&tEnd);
	float m_time = ((tEnd.QuadPart - tStart.QuadPart) / (double)(ts.QuadPart));

	app_blink_data
		<< setw(15) << setprecision(6) << m_time
		<< setw(15) << setprecision(6) << absSum
		<< endl;

}


bool EyeDetector::open_recordfile(int data_id) {
	stringstream ss;
	ss << ".//data//data" << data_id << ".txt";
	string str;
	ss >> str;

	app_blink_data.open(str.c_str(), ios::out);
	
	if (app_blink_data.is_open()) {
		QueryPerformanceFrequency(&ts);
		QueryPerformanceCounter(&tStart);
		return true;
	}
	
	return false;
}


void EyeDetector::save_eyeimage(int image_id) {
	stringstream ss;
	ss << ".//template//temp" << image_id++ << ".jpg";
	string str;
	ss >> str;

	imwrite(str.c_str(), m_eye);
}
