#include "fixes.h"

void cvCopy2(CvArr *src, CvArr *dst, CvArr *mask) {
	IplImage *tmp1dB = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dG = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dR = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);

	cvSplit(src, tmp1dB, tmp1dG, tmp1dR, NULL);

	cvAnd(tmp1dB, mask, tmp1dB);
	cvAnd(tmp1dG, mask, tmp1dG);
	cvAnd(tmp1dR, mask, tmp1dR);

	cvMerge(tmp1dB, tmp1dG, tmp1dR, NULL, dst);

	cvReleaseImage(&tmp1dB);
	cvReleaseImage(&tmp1dG);
	cvReleaseImage(&tmp1dR);
}

IplImage* createSubArray(IplImage *src, CvRect rect) {
	CvRect prevRect = cvGetImageROI(src);
	cvSetImageROI(src, rect);
	IplImage *dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	cvCopy(src, dst, NULL);
	cvSetImageROI(src, prevRect);
	return dst;
}