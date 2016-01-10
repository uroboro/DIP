#include "filter_volume.h"
#include <stdio.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>

#include "fixes.h"

int recursiveContoursDescription(IplImage *src, CvSeq *contours, int full_area, long minVolume, int indent) {
	int area_total = 0;
	for (CvSeq *tmp = contours; tmp != NULL; tmp = tmp->h_next) {
		//for (int t = 0; t < indent; t++) printf(" ");
		int area = (int)cvContourArea(tmp, CV_WHOLE_SEQ, 0);
		//printf("area:%d px (%d%%)", area, (area * 100) / full_area);

		IplImage *tmp3d = cvCloneImage(src);
		cvSet(tmp3d, cvScalarAll(0), NULL);
		cvDrawContours(tmp3d, tmp, cvScalarAll(192), cvScalarAll(64), 0, CV_FILLED, 8, cvPoint(0, 0));
		if (indent > 0) cvDrawContours(tmp3d, tmp, cvScalarAll(0), cvScalarAll(0), 0, 0, 8, cvPoint(0, 0));

		//int pixels = cvContourArea2(tmp);
		//printf(" (%d px (%d%%))", pixels, (pixels * 100) / full_area);
		cvReleaseImage(&tmp3d);

		//printf("\n");
		area_total += area;
		area_total += recursiveContoursDescription(src, tmp->v_next, full_area, minVolume, indent + 1);
	}

	return area_total;
}
int recursiveContoursDraw(IplImage *dst, CvSeq *contours, long minVolume, int indent) {
	for (CvSeq *tmp = contours; tmp != NULL; tmp = tmp->h_next) {
		int pixels = cvContourArea2(tmp);
		char condition = pixels < minVolume / 5;
		CvScalar color_fg = (condition) ? (indent % 2) ? cvScalarAll(255) : cvScalarAll(0) : (indent % 2) ? cvScalarAll(0) : cvScalarAll(255);
		CvScalar color_bg = (indent % 2) ? cvScalarAll(0) : cvScalarAll(255);
		recursiveContoursDraw(dst, tmp->v_next, pixels, indent + 1);
		cvDrawContours(dst, tmp, color_fg, color_bg, -1, (condition) ? -1 : 0, 8, cvPoint(0, 0));
	}
	return 0;
}

int filterByVolume(IplImage *src, IplImage *dst, long minVolume) {
	IplImage *tmp3d = NULL;
	IplImage *tmp1d = NULL;
	if (src->nChannels == 1) {
		tmp3d = cvCreateImage(cvGetSize(src), src->depth, 3);
		cvMerge(src, src, src, NULL, tmp3d);
		tmp1d = cvCloneImage(src);
	} else {
		tmp3d = cvCloneImage(src);
		tmp1d = cvCreateImage(cvGetSize(src), src->depth, 1);
		cvCvtColor(src, tmp1d, CV_BGR2GRAY);
	}

	//printf("total area:%d\n", src->width * src->height);
	//CVSHOW("orig0", 800, 0, 320, 240, tmp3d);

	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = NULL;
	int n = cvFindContours2(tmp1d, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	cvReleaseImage(&tmp1d);
	//printf("%d(%d) contours\n", n, contours->total);

	int area = recursiveContoursDescription(src, contours, src->width * src->height, minVolume, 0);
	//printf("counted area:%d\n", area);
	recursiveContoursDraw(tmp3d, contours, minVolume, 0);

	cvReleaseMemStorage(&storage);
	cvCvtColor(tmp3d, dst, CV_BGR2GRAY);
	cvReleaseImage(&tmp3d);
	//CVSHOW("volume", 800, 0, 320, 240, dst);
	//printf("\n");

	return 0;
}
