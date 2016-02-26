#include "fixes.h"
#include <opencv2/imgproc/imgproc_c.h>

void cvCopy2(CvArr *src, CvArr *dst, CvArr *mask) {
	IplImage *tmp1dB = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dG = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dR = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);

	cvSplit(src, tmp1dB, tmp1dG, tmp1dR, NULL);

	cvAnd(tmp1dB, mask, tmp1dB, NULL);
	cvAnd(tmp1dG, mask, tmp1dG, NULL);
	cvAnd(tmp1dR, mask, tmp1dR, NULL);

	cvMerge(tmp1dB, tmp1dG, tmp1dR, NULL, dst);

	cvReleaseImage(&tmp1dB);
	cvReleaseImage(&tmp1dG);
	cvReleaseImage(&tmp1dR);
}

void cvCopyNonZero(CvArr *src, CvArr *dst, CvArr *mask) {
	IplImage *tmp1d = cvCreateImage(cvGetSize(src), ((IplImage *)src)->depth, 1);
	cvCvtColor(src, tmp1d, CV_RGB2GRAY);
	cvCvtColor(src, src, CV_RGB2BGR);
	cvCopy(src, dst, tmp1d);
	cvReleaseImage(&tmp1d);
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
	cvResize(image, tmp3d, CV_INTER_LINEAR);
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

void cvRectangle2(CvArr* img, CvRect rect, CvScalar color, int thickness, int line_type, int shift) {
	cvRectangle(img, cvPoint(rect.x, rect.y),
			cvPoint(rect.x + rect.width - 1, rect.y + rect.height - 1),
			color, thickness, line_type, shift);
}

void cvFillConvexPoly2(CvArr* img, CvSeq *points, CvScalar color, int line_type, int shift) {
	CvPoint *pts = calloc(points->total, sizeof(CvPoint));
	if (pts) {
		for (int i = 0; i < points->total; i++) {
			pts[i] = *CV_GET_SEQ_ELEM(CvPoint, points, i);
		}
		cvFillConvexPoly(img, pts, points->total, color, line_type, shift);
		free(points);
	}
}

void cvBox2(CvArr* img, CvBox2D rect, CvScalar color, int thickness, int line_type, int shift) {
	CvPoint2D32f boxPoints[4];
	cvBoxPoints(rect, boxPoints);
	if (thickness == CV_FILLED) {
		CvPoint pts[4] = { cvPointFrom32f(boxPoints[0]), cvPointFrom32f(boxPoints[1]), cvPointFrom32f(boxPoints[2]), cvPointFrom32f(boxPoints[3]) };
		cvFillConvexPoly(img, pts, 4, color, line_type, shift);
	} else {
		cvLine(img, cvPoint((int)boxPoints[0].x, (int)boxPoints[0].y), cvPoint((int)boxPoints[1].x, (int)boxPoints[1].y), color, thickness, line_type, shift);
		cvLine(img, cvPoint((int)boxPoints[1].x, (int)boxPoints[1].y), cvPoint((int)boxPoints[2].x, (int)boxPoints[2].y), color, thickness, line_type, shift);
		cvLine(img, cvPoint((int)boxPoints[2].x, (int)boxPoints[2].y), cvPoint((int)boxPoints[3].x, (int)boxPoints[3].y), color, thickness, line_type, shift);
		cvLine(img, cvPoint((int)boxPoints[3].x, (int)boxPoints[3].y), cvPoint((int)boxPoints[0].x, (int)boxPoints[0].y), color, thickness, line_type, shift);
	}
}

void ocvResizeFrame(IplImage *src, IplImage *dst) {
		cvSetImageROI(dst, cvRect(0, 0, src->width, src->height));
		cvResize(src, dst, CV_INTER_LINEAR);
		cvResetImageROI(dst);
}

void ocv2DAffineMatrix(CvMat* map_matrix, CvPoint2D32f c, float a) {
	CV_MAT_ELEM((*map_matrix), float, 0, 0) = cos(a);
	CV_MAT_ELEM((*map_matrix), float, 0, 1) = -sin(a);
	CV_MAT_ELEM((*map_matrix), float, 0, 2) = c.x - c.x * cos(a) + c.y * sin(a);
	CV_MAT_ELEM((*map_matrix), float, 1, 0) = sin(a);
	CV_MAT_ELEM((*map_matrix), float, 1, 1) = cos(a);
	CV_MAT_ELEM((*map_matrix), float, 1, 2) = c.y - c.x * sin(a) - c.y * cos(a);
}

void cvClose(CvArr *src, CvArr *dst, CvArr *mask, size_t n) {
	cvCopy(src, dst, mask);
	//for (size_t i = 0; i < n; i++) {
		cvErode(dst, dst, NULL, n);
		cvDilate(dst, dst, NULL, n);
	//}
}
