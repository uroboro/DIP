#include "filter_hsv.h"
#include <stdio.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>

#include "fixes.h"

void cvClose(CvArr *src, CvArr *dst, CvArr *mask, size_t n);
#if SAVE_MASK
IplImage *tmp_mask = NULL;
void mouseCallback2(int event, int x, int y, int flags, void* userdata) {
	if (event == CV_EVENT_LBUTTONDOWN && tmp_mask != NULL) {
		printf("saving to resources/mask.png\n");
		cvSaveImage("resources/mask.png", tmp_mask);
	}
}
#endif

int filterByHSV(IplImage *src, CvScalar minHSV, CvScalar maxHSV, IplImage *dst) {
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
		cvInRangeS(tmp1dH_mask, cvScalar(minHSV.val[0], 0, 0), cvScalar(maxHSV.val[0], 0, 0), tmp1dH_mask);
	} else {
		IplImage *tmp1d = cvCloneImage(tmp1dH_mask);
		cvInRangeS(tmp1dH_mask, cvScalar(0, 0, 0), cvScalar(maxHSV.val[0], 0, 0), tmp1d);
		cvInRangeS(tmp1dH_mask, cvScalar(minHSV.val[0], 0, 0), cvScalar(255, 0, 0), tmp1dH_mask);
		cvOr(tmp1d, tmp1dH_mask, tmp1dH_mask, NULL);
		cvReleaseImage(&tmp1d);
	}

	cvInRangeS(tmp1dS_mask, cvScalar(minHSV.val[1], 0, 0), cvScalar(maxHSV.val[1], 0, 0), tmp1dS_mask);
	cvInRangeS(tmp1dV_mask, cvScalar(minHSV.val[2], 0, 0), cvScalar(maxHSV.val[2], 0, 0), tmp1dV_mask);

	IplImage *tmp1d_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvSet(tmp1d_mask, cvScalarAll(255), NULL);
	cvAnd(tmp1d_mask, tmp1dH_mask, tmp1d_mask, NULL);
	cvAnd(tmp1d_mask, tmp1dS_mask, tmp1d_mask, NULL);
	cvAnd(tmp1d_mask, tmp1dV_mask, tmp1d_mask, NULL);

	cvReleaseImage(&tmp1dH_mask);
	cvReleaseImage(&tmp1dS_mask);
	cvReleaseImage(&tmp1dV_mask);

	cvClose(tmp1d_mask, tmp1d_mask, NULL, 2);

#define CONTROL_WINDOW	"Control Window"
#define CONTROLS_WIDTHA  640/2
#define CONTROLS_HEIGHTA 480/2
#if 1
	cvNamedWindow(CONTROL_WINDOW, 0);
#if SAVE_MASK
	if (tmp_mask == NULL) {
		tmp_mask = cvCloneImage(tmp1d_mask);
	} else {
		cvCopy(tmp1d_mask, tmp_mask, NULL);
	}
	cvSetMouseCallback(CONTROL_WINDOW, mouseCallback2, NULL);
#endif
	cvResizeWindow(CONTROL_WINDOW, CONTROLS_WIDTHA, CONTROLS_HEIGHTA);
	cvShowImage(CONTROL_WINDOW, tmp1d_mask);
#endif

	cvCopy2(src, dst, tmp1d_mask);

	cvReleaseImage(&tmp1d_mask);

	return 0;
}
