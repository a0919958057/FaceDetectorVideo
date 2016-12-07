#include<iostream>
#include<opencv2\opencv.hpp>
#include <tchar.h>
#include <windows.h>
#include"EyeDetector.h"

int main(int argc, char** argv) {

	EyeDetector detector;

	String path_file("C:\\Users\\TwnET\\Desktop\\FaceSample\\V_20161025_182145.mp4");

	VideoCapture cap(path_file);
	detector.init(cap, Size(640,360),RotFlag::ROT_CW_90);

	CascadeClassifier faceCascade("cascadeFile.xml");
	CascadeClassifier eyeCascade("ojoD.xml");

	std::vector<Rect> face_vec;
	std::vector<Rect> eye_vec;

	detector.set_classifier(eyeCascade,eye_vec);

	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;

	if ( QueryPerformanceCounter(&StartingTime) != 0) {

	}

	while (waitKey(1) != 'a') {
		//QueryPerformanceCounter(&StartingTime);
		detector.update_image();
		//QueryPerformanceCounter(&EndingTime);
		//QueryPerformanceFrequency(&ElapsedMicroseconds);

		//cout << "Update image delay " 
		//	<< 1.0/(((double)EndingTime.QuadPart - (double)StartingTime.QuadPart) / 
		//		(double)ElapsedMicroseconds.QuadPart) 
		//	<< endl;

		detector.detect_eye();
		detector.show_frame();

		detector.record_data(0);

	}

	system("pause");
	

	return 0;
}

