#include "filter_grayscale.h"

int maskByDistance2Grayscale(IplImage *src, IplImage *dst, int minDistance) {
	IplImage *tmp3d = cvCreateImage(cvGetSize(src), src->depth, 3);
	ocvDistance2Grayscale(src, tmp3d);
	cvThreshold(tmp3d, tmp3d, minDistance, 255, CV_THRESH_BINARY);
	cvCvtColor(tmp3d, dst, CV_BGR2GRAY);
	cvReleaseImage(&tmp3d);
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
