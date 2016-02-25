#include "filter_hsv.h"
#include <stdio.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>

#include "fixes.h"

#define CONTROL_WINDOW	"HSV Control Window"
#define CONTROLS_WIDTH 400
#define CONTROLS_HEIGHT 100

static void trackbarCallback(int val) {}

static void setupWindows(int *a, int *b, int *c, int *d, int *e, int *f) {
	cvNamedWindow(CONTROL_WINDOW  "1", 0);
	cvResizeWindow(CONTROL_WINDOW "1", CONTROLS_WIDTH, CONTROLS_HEIGHT);

	cvCreateTrackbar("min 0", CONTROL_WINDOW "1", a, 180, trackbarCallback);
	cvCreateTrackbar("min 1", CONTROL_WINDOW "1", b, 255, trackbarCallback);
	cvCreateTrackbar("min 2", CONTROL_WINDOW "1", c, 255, trackbarCallback);

	cvNamedWindow(CONTROL_WINDOW  "2", 0);
	cvResizeWindow(CONTROL_WINDOW "2", CONTROLS_WIDTH, CONTROLS_HEIGHT);

	cvCreateTrackbar("max 0", CONTROL_WINDOW "2", d, 180, trackbarCallback);
	cvCreateTrackbar("max 1", CONTROL_WINDOW "2", e, 255, trackbarCallback);
	cvCreateTrackbar("max 2", CONTROL_WINDOW "2", f, 255, trackbarCallback);
}

static void destroyWindows(void) {
	cvDestroyWindow(CONTROL_WINDOW "1");
	cvDestroyWindow(CONTROL_WINDOW "2");
}

DIP_EXTERN int maskByHSV(IplImage *src, CvScalar minHSV, CvScalar maxHSV, IplImage *dst_mask) {
	IplImage *tmp3d = cvCloneImage(src);
	cvSmooth(tmp3d, tmp3d, CV_GAUSSIAN, 9, 0, 0, 0);

	cvCvtColor(tmp3d, tmp3d, CV_BGR2HSV);
	IplImage *tmp1dH_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dS_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dV_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvSplit(tmp3d, tmp1dH_mask, tmp1dS_mask, tmp1dV_mask, NULL);
	cvReleaseImage(&tmp3d);

	//printf("\rmin: %03d,%03d,%03d", (int)minHSV.val[0], (int)minHSV.val[1], (int)minHSV.val[2]);
	//printf("\tmax: %03d,%03d,%03d", (int)maxHSV.val[0], (int)maxHSV.val[1], (int)maxHSV.val[2]);

	if (minHSV.val[0] < maxHSV.val[0]) {
		cvInRangeS(tmp1dH_mask, cvScalarAll(minHSV.val[0]), cvScalarAll(maxHSV.val[0]), tmp1dH_mask);
	} else {
		IplImage *tmp1d = cvCloneImage(tmp1dH_mask);
		cvInRangeS(tmp1dH_mask, cvScalarAll(0), cvScalarAll(maxHSV.val[0]), tmp1d);
		cvInRangeS(tmp1dH_mask, cvScalarAll(minHSV.val[0]), cvScalarAll(255), tmp1dH_mask);
		cvOr(tmp1d, tmp1dH_mask, tmp1dH_mask, NULL);
		cvReleaseImage(&tmp1d);
	}

	cvInRangeS(tmp1dS_mask, cvScalarAll(minHSV.val[1]), cvScalarAll(maxHSV.val[1]), tmp1dS_mask);
	cvInRangeS(tmp1dV_mask, cvScalarAll(minHSV.val[2]), cvScalarAll(maxHSV.val[2]), tmp1dV_mask);

	IplImage *tmp1d_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvSet(tmp1d_mask, cvScalarAll(255), NULL);
	cvAnd(tmp1d_mask, tmp1dH_mask, tmp1d_mask, NULL);
	cvAnd(tmp1d_mask, tmp1dS_mask, tmp1d_mask, NULL);
	cvAnd(tmp1d_mask, tmp1dV_mask, tmp1d_mask, NULL);

	cvReleaseImage(&tmp1dH_mask);
	cvReleaseImage(&tmp1dS_mask);
	cvReleaseImage(&tmp1dV_mask);

	cvCopy(tmp1d_mask, dst_mask, NULL);
	cvReleaseImage(&tmp1d_mask);
	//CVSHOW("hsv mask", 600, 0, 320, 240, dst_mask);
	//cvSaveImage("Resources/mask.png", dst_mask, NULL);

	return 0;
}

int filterByHSV(IplImage *src, CvScalar minHSV, CvScalar maxHSV, IplImage *dst) {
	IplImage *tmp1d_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	maskByHSV(src, minHSV, maxHSV, tmp1d_mask);

	cvCopy2(src, dst, tmp1d_mask);
	cvReleaseImage(&tmp1d_mask);
	//CVSHOW("hsv ", 600, 300, 320, 240, dst);

	return 0;
}
