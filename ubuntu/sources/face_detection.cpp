#include "face_detection.h"
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/objdetect/objdetect.hpp>

#include "fixes.h"

int findFaces(CvArr* src, CvRect **rects, double scale, double *t) {
	// Make image grayscale
	IplImage *tmp1d = NULL;
	if (((IplImage *)src)->nChannels == 1) {
		tmp1d = cvCloneImage((IplImage *)src);
	} else {
		tmp1d = cvCreateImage(cvGetSize(src), ((IplImage *)src)->depth, 1);
		cvCvtColor(src, tmp1d, CV_BGR2GRAY);
	}

	// Scale down image
	CvSize srcSize = cvGetSize(src);
	double fx = 1 / scale;
	srcSize.width = (int)cvRound(fx * srcSize.width);
	srcSize.height = (int)cvRound(fx * srcSize.height);

	IplImage *smallImg = cvCreateImage(srcSize, IPL_DEPTH_8U, 1);
	cvResize(tmp1d, smallImg, CV_INTER_LINEAR);
	cvReleaseImage(&tmp1d);

	cvEqualizeHist(smallImg, smallImg);

	double scale_factor = 1.1;
	int min_neighbors = 3;
	int flags = 0
		//|CASCADE_FIND_BIGGEST_OBJECT
		//|CASCADE_DO_ROUGH_SEARCH
		| cv::CASCADE_SCALE_IMAGE;
	CvSize min_size = cvSize(0,0);
	CvSize max_size = cvSize(100,100);
	std::vector<cv::Rect> objects;
	static cv::CascadeClassifier cascade;
	if (cascade.empty() && !cascade.load("Resources/haarcascade_frontalface_alt.xml")) {
		printf("ERROR: Could not load classifier cascade\n");
		return -1;
	}
	double t_diff = (double)cvGetTickCount();
	cascade.detectMultiScale(smallImg, objects, scale_factor, min_neighbors, flags, min_size, max_size);
	*t = ((double)cvGetTickCount() - t_diff) / (cvGetTickFrequency() * 1000);
	cvReleaseImage(&smallImg);

	int size = objects.size();
	*rects = (CvRect *)calloc(size + 1, sizeof(CvRect));
	for (int i = 0; i < size; i++) {
		CvRect face = objects[i];
		(*rects)[i] = cvRect(cvRound(face.x * scale), cvRound(face.y * scale), cvRound(face.width * scale), cvRound(face.height * scale));
	}

	return size;
}

void drawFaces(CvArr *dst, size_t facesCount, CvRect *faces) {
	const static CvScalar colors[] =
	{
		cvScalar(255, 000, 000),
		cvScalar(255, 128, 000),
		cvScalar(255, 255, 000),
		cvScalar(000, 255, 000),
		cvScalar(000, 128, 255),
		cvScalar(000, 255, 255),
		cvScalar(000, 000, 255),
		cvScalar(255, 000, 255)
	};

	for (size_t i = 0; i < facesCount; i++) {
		cvRectangle2(dst, faces[i], colors[i % 8], 3, 8, 0);
	}
}
