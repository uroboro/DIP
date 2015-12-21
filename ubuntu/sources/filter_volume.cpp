#include "filter_volume.h"
#include <stdio.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>

#define CVSHOW(name, x, y, image) { cvNamedWindow(name, 0); cvMoveWindow(name, x, y); cvShowImage(name, image); }

int recursiveContoursDescription(CvSeq *contours, int full_area, int indent) {
	int area_total = 0;
	int total = 0; for (CvSeq *tmp = contours; tmp != NULL; tmp = tmp->h_next, total++);
	CvSeq *tmp = contours;
	for (int i = 0; tmp != NULL; tmp = tmp->h_next, i++) {
		for (int t = 0; t < indent; t++) printf(" ");
		int area = (int)cvContourArea(tmp, CV_WHOLE_SEQ, 0);
		printf("%d area:%d%%\n", i, (area * 100)/full_area);
		area_total += area;
		area_total += recursiveContoursDescription(tmp->v_next, full_area, indent + 1);
	}
	return area_total;
}
int recursiveContoursDraw(IplImage *dst, CvSeq *contours, int indent) {
	int total = 0; for (CvSeq *tmp = contours; tmp != NULL; tmp = tmp->h_next, total++);
	CvSeq *tmp = contours;
	for (int i = 0; tmp != NULL; tmp = tmp->h_next, i++) {
		CvScalar colorpx1 = CV_RGB(32 + 32 * indent, 32 + 32 * i, 0);
		CvScalar colorpx2 = CV_RGB(127,127,127);//CV_RGB(0, 64, 16 * indent + cc % 128);
		recursiveContoursDraw(dst, tmp->v_next, indent + 1);
		cvDrawContours(dst, tmp, colorpx1, colorpx2, -1, 0, 8, cvPoint(0, 0));
	}
	return 0;
}

int filterByVolume(IplImage *src, IplImage *dst, long minVolume) {
	printf("total area:%d\n", src->width * src->height);
{
	IplImage *tmp3d = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvMerge(src, src, src, NULL, tmp3d);
	CVSHOW("orig0", 0, 0, tmp3d);

	IplImage *tmp1d = cvCloneImage(src);
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = NULL;
	int n = cvFindContours2(tmp1d, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	printf("%d(%d) contours\n", n, contours->total);
	cvReleaseImage(&tmp1d);

	int area = recursiveContoursDescription(contours, src->width * src->height, 0);
	printf("counted area:%d\n", area);
	recursiveContoursDraw(tmp3d, contours, 1);

	cvReleaseMemStorage(&storage);
	CVSHOW("orig1", 0, 300, tmp3d);
	cvReleaseImage(&tmp3d);
	printf("\n");
}
if (01)
{
	IplImage *tmp3d = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvMerge(src, src, src, NULL, tmp3d);
	cvThreshold(tmp3d, tmp3d, 127, 255, CV_THRESH_BINARY_INV);
	CVSHOW("flip0", 600, 0, tmp3d);

	IplImage *tmp1d = cvCloneImage(src);
	cvThreshold(tmp1d, tmp1d, 127, 255, CV_THRESH_BINARY_INV);
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = NULL;
	int n = cvFindContours2(tmp1d, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	printf("%d(%d) contours\n", n, contours->total);
	cvReleaseImage(&tmp1d);

	int area = recursiveContoursDescription(contours, src->width * src->height, 0);
	printf("counted area:%d\n", area);
	recursiveContoursDraw(tmp3d, contours, 1);

	cvReleaseMemStorage(&storage);
	CVSHOW("flip1", 600, 300, tmp3d);
	cvReleaseImage(&tmp3d);
	printf("\n");
}

	return 0;
}
