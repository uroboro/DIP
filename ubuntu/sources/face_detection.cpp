#include "face_detection.h"
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/objdetect/objdetect.hpp>

#ifdef __WIN__
#define RESOURCES "C:\\Users\\uroboro\\Documents\\GitHub\\DIP\\DIP\\resources\\"
#else
#define RESOURCES "/home/uroboro/Documents/GitHub/DIP/DIP/resources/"
#endif
#define LS printf("%s::%d\n", __PRETTY_FUNCTION__, __LINE__);

int findFaces(CvArr* src, CvRect **rects, double scale, double *t) {
	CvSize srcSize = cvGetSize(src);
	IplImage *gray = cvCreateImage(srcSize, IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);

	double fx = 1 / scale;
	srcSize.width = (int)cvRound(fx * srcSize.width);
	srcSize.height = (int)cvRound(fx * srcSize.height);

	IplImage *smallImg = cvCreateImage(srcSize, IPL_DEPTH_8U, 1);
	cvResize(gray, smallImg, CV_INTER_LINEAR);
	cvReleaseImage(&gray);

	cvEqualizeHist(smallImg, smallImg);

	char frontalFaceFile[] = RESOURCES "haarcascade_frontalface_alt.xml";

LS;	cv::CascadeClassifier cascade;
	if (!cascade.load(frontalFaceFile)) {
		printf("ERROR: Could not load classifier cascade\r");
		return -1;
	}

	std::vector<cv::Rect> faces;
	*t = (double)cvGetTickCount();
LS;	cascade.detectMultiScale(smallImg, faces,
		1.1, 2, 0
		//|CASCADE_FIND_BIGGEST_OBJECT
		//|CASCADE_DO_ROUGH_SEARCH
		| cv::CASCADE_SCALE_IMAGE,
		cv::Size(30, 30));

	*t = (double)cvGetTickCount() - *t;
	*t /= ((double)cvGetTickFrequency() * 1000);

	*rects = (CvRect *)calloc(faces.size() + 1, sizeof(CvRect));
LS;	for (size_t i = 0; i < faces.size(); i++) {
		CvRect face = faces[i];
		face = cvRect(cvRound(face.x * scale), cvRound(face.y * scale), cvRound(face.width * scale), cvRound(face.height * scale));
		(*rects)[i] = face;
LS;	}

LS;	int size = faces.size();
LS;	return size;
}

void drawFaces(CvArr *dst, size_t facesCount, CvRect *faces) {
	const static CvScalar colors[] =
	{
		cvScalar(255, 0, 0),
		cvScalar(255, 128, 0),
		cvScalar(255, 255, 0),
		cvScalar(0, 255, 0),
		cvScalar(0, 128, 255),
		cvScalar(0, 255, 255),
		cvScalar(0, 0, 255),
		cvScalar(255, 0, 255)
	};

	for (size_t i = 0; i < facesCount; i++) {
		CvRect r = faces[i];
		CvScalar color = colors[i % 8];

		cvRectangle(dst, cvPoint(r.x, r.y),
			cvPoint(r.x + r.width - 1, r.y + r.height - 1),
			color, 3, 8, 0);
	}
}
