#include "filter_volume.h"
#include <stdio.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>

#include "fixes.h"

#define CVSHOW(name, x, y, w, h, image) { cvNamedWindow(name, 0); cvMoveWindow(name, x, y); cvResizeWindow(name, w, h); cvShowImage(name, image); }

int aaaaaa;
int recursiveContoursDescription(IplImage *src, CvSeq *contours, int full_area, long minVolume, int indent) {
	int area_total = 0;
	int total = 0; for (CvSeq *tmp = contours; tmp != NULL; tmp = tmp->h_next, total++);
	CvSeq *tmp = contours;

	for (int i = 0; tmp != NULL; tmp = tmp->h_next, i++) {
		for (int t = 0; t < indent; t++) printf(" ");
		int area = (int)cvContourArea(tmp, CV_WHOLE_SEQ, 0);
		printf("%d area:%d px (%d%%)", i, area, (area * 100) / full_area);

		if (1) {
			IplImage *tmp3d = cvCloneImage(src);
			cvSet(tmp3d, cvScalarAll(0), NULL);
			cvDrawContours(tmp3d, tmp, cvScalarAll(192), cvScalarAll(64), 0, CV_FILLED, 8, cvPoint(0, 0));
			char varname[] = "X_X_X";
			varname[0] = '0' + aaaaaa;
			varname[2] = '0' + indent;
			varname[4] = '0' + i;
			if (indent > 0) cvDrawContours(tmp3d, tmp, cvScalarAll(0), cvScalarAll(0), 0, 0, 8, cvPoint(0, 0));
			//CVSHOW(varname, 150+300 * aaaaaa+10*indent, 100+50 * i, 200, 200, tmp3d);
			int pixels = cvContourArea2(tmp);
			printf(" (%d px (%d%%))", pixels, (pixels * 100) / full_area);
			cvReleaseImage(&tmp3d);
		}
		printf("\n");
		area_total += area;
		area_total += recursiveContoursDescription(src, tmp->v_next, full_area, minVolume, indent + 1);
	}

	return area_total;
}
int recursiveContoursDraw(IplImage *dst, CvSeq *contours, long minVolume, int indent) {
	int total = 0; for (CvSeq *tmp = contours; tmp != NULL; tmp = tmp->h_next, total++);
	CvSeq *tmp = contours;
	for (int i = 0; tmp != NULL; tmp = tmp->h_next, i++) {
		int pixels = cvContourArea2(tmp);
		CvScalar colorpx1 = (pixels < minVolume) ? CV_RGB(0, 0, 0) : CV_RGB(0, 255, 0);
		CvScalar colorpx2 = CV_RGB(127,127,127);//CV_RGB(0, 64, 16 * indent + cc % 128);
		recursiveContoursDraw(dst, tmp->v_next, minVolume, indent + 1);
		cvDrawContours(dst, tmp, colorpx1, colorpx2, -1, (pixels < minVolume) ? -1 : 0, 8, cvPoint(0, 0));
	}
	return 0;
}

int filterByVolume(IplImage *src, IplImage *dst, long minVolume) {
	printf("total area:%d\n", src->width * src->height);
{
	IplImage *tmp3d = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvMerge(src, src, src, NULL, tmp3d);
	CVSHOW("orig0", 0, 0, 200, 200, tmp3d);
aaaaaa = 0;
	IplImage *tmp1d = cvCloneImage(src);
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = NULL;
	int n = cvFindContours2(tmp1d, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	printf("%d(%d) contours\n", n, contours->total);
	cvReleaseImage(&tmp1d);

	int area = recursiveContoursDescription(src, contours, src->width * src->height, minVolume, 0);
	printf("counted area:%d\n", area);
	recursiveContoursDraw(tmp3d, contours, minVolume, 1);

	cvReleaseMemStorage(&storage);
	CVSHOW("orig1", 0, 250, 200, 200, tmp3d);
	cvReleaseImage(&tmp3d);
	printf("\n");
}
if (0)
{
	IplImage *tmp3d = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvMerge(src, src, src, NULL, tmp3d);
	cvThreshold(tmp3d, tmp3d, 127, 255, CV_THRESH_BINARY_INV);
	CVSHOW("flip0", 500, 0, 200, 200, tmp3d);
aaaaaa = 1;
	IplImage *tmp1d = cvCloneImage(src);
	cvThreshold(tmp1d, tmp1d, 127, 255, CV_THRESH_BINARY_INV);
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = NULL;
	int n = cvFindContours2(tmp1d, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	printf("%d(%d) contours\n", n, contours->total);
	cvReleaseImage(&tmp1d);

	int area = recursiveContoursDescription(src, contours, src->width * src->height, minVolume, 0);
	printf("counted area:%d\n", area);
	recursiveContoursDraw(tmp3d, contours, minVolume, 1);

	cvReleaseMemStorage(&storage);
	CVSHOW("flip1", 500, 250, 200, 200, tmp3d);
	cvReleaseImage(&tmp3d);
	printf("\n");
}

	return 0;
}
