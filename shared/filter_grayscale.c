#include "filter_grayscale.h"

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
