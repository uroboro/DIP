#include "filter_grayscale.h"

int maskByDistance2Grayscale(IplImage *src, IplImage *dst, int minDistance) {
	IplImage *tmp3d = cvCreateImage(cvGetSize(src), src->depth, 3);
	ocvDistance2Grayscale(src, tmp3d);
	cvSmooth(tmp3d, tmp3d, CV_GAUSSIAN, 9, 0, 0, 0);
	cvCvtColor(tmp3d, dst, CV_RGB2GRAY);
	cvReleaseImage(&tmp3d);
	cvEqualizeHist(dst, dst);
	//CVSHOW("grayscale2", dst->width*2/3, dst->height*2/3, dst->width/2, dst->height/2, dst);
	//cvThreshold(dst, dst, minDistance, 255, CV_THRESH_BINARY);
	return 0;
}

int ocvDistance2Grayscale(IplImage *src, IplImage *dst) {
	if (!src || !dst) { /*present(1, "!image"); */return 1; }

	if (src->nChannels == 3) {
		for (unsigned int y = 0; y < src->height; y++) {
			for (unsigned int x = 0; x < src->width; x++) {
				CvScalar p = cvGet2D(src, y, x);
				// Calculate distance to grayscale value
				cvSet2D(dst, y, x, cvScalarAll(sqrt(2.0 / 3 * (
					  p.val[0] * (p.val[0] - p.val[1])
					+ p.val[1] * (p.val[1] - p.val[2])
					+ p.val[2] * (p.val[2] - p.val[0])
				))));
			}
		}
	}

	return 0;
}
