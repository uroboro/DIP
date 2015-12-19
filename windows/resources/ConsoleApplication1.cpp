// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <opencv2/core/core_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect_c.h>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>

using namespace std;
using namespace cv;

#define TRY(X) { char $E = 0; try { X } catch (cv::Exception *e) { std::cout << e->what(); $E = 1; } $E; }



static void help()
{
	cout << "\nThis program demonstrates the cascade recognizer. Now you can use Haar or LBP features.\n"
		"This classifier can recognize many kinds of rigid objects, once the appropriate classifier is trained.\n"
		"It's most known use is for faces.\n"
		"Usage:\n"
		"./facedetect [--cascade=<cascade_path> this is the primary trained classifier such as frontal face]\n"
		"   [--nested-cascade[=nested_cascade_path this an optional secondary classifier such as eyes]]\n"
		"   [--scale=<image scale greater or equal to 1, try 1.3 for example>]\n"
		"   [--try-flip]\n"
		"   [filename|camera_index]\n\n"
		"see facedetect.cmd for one call:\n"
		"./facedetect --cascade=\"../../data/haarcascades/haarcascade_frontalface_alt.xml\" --nested-cascade=\"../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xml\" --scale=1.3\n\n"
		"During execution:\n\tHit any key to quit.\n"
		"\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
}


void detectAndDraw(Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade, Mat& dst, double scale, bool tryflip);

string cascadeName = "C:\\Users\\uroboro\\Documents\\GitHub\\DIP\\DIP\\resources\\haarcascade_frontalface_alt.xml";
string nestedCascadeName = "C:\\Users\\uroboro\\Documents\\GitHub\\DIP\\DIP\\resources\\haarcascade_eye.xml";

int _tmain(int argc, char *argv[]) {
	VideoCapture capture;
	Mat frame, image;
	const string scaleOpt = "--scale";
	size_t scaleOptLen = scaleOpt.length();
	const string tryFlipOpt = "--try-flip";
	size_t tryFlipOptLen = tryFlipOpt.length();
	string inputName;
	bool tryflip = false;

	help();

	CascadeClassifier cascade, nestedCascade;
	double scale = 1;

	if (!cascade.load(cascadeName)) {
		cerr << "ERROR: Could not load classifier cascade" << endl;
		help();
		return -1;
	}
	if (!nestedCascade.load(nestedCascadeName)) {
		cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
		nestedCascade = NULL;
	}

	string facesName = "C:\\Users\\uroboro\\Downloads\\faces.png";
	image = imread(facesName, 1);
	if (image.empty()) {
		cout << "Couldn't read " << facesName << endl;
	}


	cout << "Detecting face(s) in " << inputName << endl;
	if (0&& !image.empty()) {
		Mat dst(image);
		detectAndDraw(image, cascade, nestedCascade, dst, scale, tryflip);
		IplImage* img = new IplImage(dst);
		cvNamedWindow("AAA", WINDOW_AUTOSIZE);
		cvShowImage("AAA", img);
		//cvReleaseImage(&img);
		waitKey(0);
		cvDestroyWindow("AAA");
	}

	int key = 0;
	CvCapture *cv_cap = cvCaptureFromCAM(CV_CAP_ANY);
	if (!cv_cap) {
		std::cout << "Could not open camera\n";
		while ((key = cvWaitKey(20)) != 'q') {
			cout << "waiting...\r";
		}
		return -1;
	}

	int cam_width = (int)cvGetCaptureProperty(cv_cap, CV_CAP_PROP_FRAME_WIDTH);
	int cam_height = (int)cvGetCaptureProperty(cv_cap, CV_CAP_PROP_FRAME_HEIGHT);
	CvSize cam_size = cvSize(cam_width, cam_height);

	while ((key = cvWaitKey(20)) != 27) {
		IplImage *cam = cvQueryFrame(cv_cap); // get frame
		if (!cam) {
			std::cout << "no input\n";
			continue;
		}
		Mat dst = cvarrToMat(cam);
		detectAndDraw(dst, cascade, nestedCascade, dst, scale, tryflip);
		IplImage* img = new IplImage(dst);
		cvNamedWindow("AAA", WINDOW_AUTOSIZE);
		cvShowImage("AAA", img);
		cvDestroyWindow("AAA");
	}

	while ((key = cvWaitKey(20)) != 'q') {
		cout << "waiting...\r";
	}
	return 0;
}

void *findFaces(CvArr* src) {
	CvRect r;

}


void detectAndDraw(Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade, Mat& dst, double scale, bool tryflip)
{
	const static Scalar colors[] =
	{
		Scalar(255, 0, 0),
		Scalar(255, 128, 0),
		Scalar(255, 255, 0),
		Scalar(0, 255, 0),
		Scalar(0, 128, 255),
		Scalar(0, 255, 255),
		Scalar(0, 0, 255),
		Scalar(255, 0, 255)
	};
	Mat gray, smallImg;

	cvtColor(img, gray, COLOR_BGR2GRAY);
	double fx = 1 / scale;
	resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR);
	equalizeHist(smallImg, smallImg);

	vector<Rect> faces, faces2;
	double t = 0;
	t = (double)cvGetTickCount();
	cascade.detectMultiScale(smallImg, faces,
		1.1, 2, 0
		//|CASCADE_FIND_BIGGEST_OBJECT
		//|CASCADE_DO_ROUGH_SEARCH
		| CASCADE_SCALE_IMAGE,
		Size(30, 30));
	if (tryflip) {
		flip(smallImg, smallImg, 1);
		cascade.detectMultiScale(smallImg, faces2,
			1.1, 2, 0
			//|CASCADE_FIND_BIGGEST_OBJECT
			//|CASCADE_DO_ROUGH_SEARCH
			| CASCADE_SCALE_IMAGE,
			Size(30, 30));
		for (vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); r++) {
			faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
		}
	}

	t = (double)cvGetTickCount() - t;
	printf("detection time = %g ms\n", t / ((double)cvGetTickFrequency()*1000.));
	for (size_t i = 0; i < faces.size(); i++) {
		Rect r = faces[i];
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center;
		Scalar color = colors[i % 8];
		int radius;

		double aspect_ratio = (double)r.width / r.height;
		if (0.75 < aspect_ratio && aspect_ratio < 1.3) {
			center.x = cvRound((r.x + r.width*0.5)*scale);
			center.y = cvRound((r.y + r.height*0.5)*scale);
			radius = cvRound((r.width + r.height)*0.25*scale);
			circle(img, center, radius, color, 3, 8, 0);
		} else {
			rectangle(img, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)),
				cvPoint(cvRound((r.x + r.width - 1)*scale), cvRound((r.y + r.height - 1)*scale)),
				color, 3, 8, 0);
		}
		if (nestedCascade.empty()) {
			continue;
		}
		smallImgROI = smallImg(r);
		nestedCascade.detectMultiScale(smallImgROI, nestedObjects,
			1.1, 2, 0
			//|CASCADE_FIND_BIGGEST_OBJECT
			//|CASCADE_DO_ROUGH_SEARCH
			//|CASCADE_DO_CANNY_PRUNING
			| CASCADE_SCALE_IMAGE,
			Size(30, 30));
		for (size_t j = 0; j < nestedObjects.size(); j++) {
			Rect nr = nestedObjects[j];
			center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
			center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
			radius = cvRound((nr.width + nr.height)*0.25*scale);
			circle(img, center, radius, color, 3, 8, 0);
		}
	}
}