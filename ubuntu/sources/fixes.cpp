#include "fixes.h"
#include <opencv2/imgproc/imgproc_c.h>

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

void cvTranslateImage2(IplImage *src, IplImage *dst, int offsetx, int offsety) {
	CvMat* map_matrix = cvCreateMat(2, 3, CV_32FC1);
	CV_MAT_ELEM((*map_matrix), float, 0, 0) = 1;
	CV_MAT_ELEM((*map_matrix), float, 0, 1) = 0;
	CV_MAT_ELEM((*map_matrix), float, 0, 2) = offsetx;
	CV_MAT_ELEM((*map_matrix), float, 1, 0) = 0;
	CV_MAT_ELEM((*map_matrix), float, 1, 1) = 1;
	CV_MAT_ELEM((*map_matrix), float, 1, 2) = offsety;

	cvWarpAffine(src, dst, map_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
	return;
}

static void cvFindContours2FixOffset(CvSeq *contours, CvPoint offset) {
	CvSeq *tmp = contours;
	for (int i = 0; tmp != NULL; tmp = tmp->h_next, i++) {
		for (int t = 0; t < tmp->total; t++) {
			CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, tmp, t);
			p->x += offset.x; p->y += offset.y;
		}
		cvFindContours2FixOffset(tmp->v_next, offset);
	}
	return;
}

int cvFindContours2(IplImage* image, CvMemStorage* storage, CvSeq** first_contour, int header_size, int mode, int method, CvPoint offset) {
	CvSize size = cvGetSize(image);
	size.width += 2; size.height += 2;
	IplImage *tmp3d = cvCreateImage(size, image->depth, image->nChannels);

	CvRect rect = cvRect(0, 0, image->width, image->height);
	cvSetImageROI(tmp3d, rect);
	cvResize(image, tmp3d);
	cvResetImageROI(tmp3d);
	cvTranslateImage2(tmp3d, tmp3d, 1, 1);

	int n = cvFindContours(tmp3d, storage, first_contour, header_size, mode, method, offset);
	cvReleaseImage(&tmp3d);
	cvFindContours2FixOffset(*first_contour, cvPoint(-1, -1));
	return n;
}

int cvContourArea2(CvSeq *contour) {
	IplImage *tmp3d = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
	cvSet(tmp3d, cvScalarAll(0), NULL);
	cvDrawContours(tmp3d, contour, cvScalarAll(255), cvScalarAll(255), 0, CV_FILLED, 8, cvPoint(0, 0));
	int pixels = cvCountNonZero(tmp3d);
	cvReleaseImage(&tmp3d);
	return pixels;
}

IplImage* createSubArray(IplImage *src, CvRect rect) {
	CvRect prevRect = cvGetImageROI(src);
	cvSetImageROI(src, rect);
	IplImage *dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	cvCopy(src, dst, NULL);
	cvSetImageROI(src, prevRect);
	return dst;
}