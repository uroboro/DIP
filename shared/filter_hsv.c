#include "filter_hsv.h"

#include "fixes.h"

DIP_EXTERN int maskByHSV(IplImage *src, IplImage *dst, CvScalar minHSV, CvScalar maxHSV) {
	IplImage *tmp1dH_mask = cvCreateImage(cvGetSize(src), src->depth, 1);
	IplImage *tmp1dS_mask = cvCreateImage(cvGetSize(src), src->depth, 1);
	IplImage *tmp1dV_mask = cvCreateImage(cvGetSize(src), src->depth, 1);

	{ // Get HSV values
		IplImage *tmp3d = cvCreateImage(cvGetSize(src), src->depth, 3);
		cvCopy(src, tmp3d, NULL);
		cvSmooth(tmp3d, tmp3d, CV_GAUSSIAN, 9, 0, 0, 0);
		cvCvtColor(tmp3d, tmp3d, CV_RGB2HSV);
		cvSplit(tmp3d, tmp1dH_mask, tmp1dS_mask, tmp1dV_mask, NULL);
		cvReleaseImage(&tmp3d);
	}

	// Filter each range separately since cvInRange doesn't handle low limit > high limit (special case for hue range)
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

	// Generate mask
	IplImage *tmp3d = cvCreateImage(cvGetSize(src), src->depth, 3);
	cvMerge(tmp1dH_mask, tmp1dS_mask, tmp1dV_mask, NULL, tmp3d);
	cvCvtColor(tmp3d, dst, CV_RGB2GRAY);
	cvThreshold(dst, dst, 240, 255, CV_THRESH_BINARY);
	cvReleaseImage(&tmp3d);

	cvReleaseImage(&tmp1dH_mask);
	cvReleaseImage(&tmp1dS_mask);
	cvReleaseImage(&tmp1dV_mask);

	return 0;
}

int filterByHSV(IplImage *src, IplImage *dst, CvScalar minHSV, CvScalar maxHSV) {
	IplImage *tmp1d_mask = cvCreateImage(cvGetSize(src), src->depth, 1);
	maskByHSV(src, tmp1d_mask, minHSV, maxHSV);
	cvCopy2(src, dst, tmp1d_mask);
	cvReleaseImage(&tmp1d_mask);

	return 0;
}
