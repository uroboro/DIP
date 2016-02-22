#include "ocv_cuantify.h"

static inline unsigned char cuantifyEquation(unsigned char p, unsigned int colorDepth) {
	double r = (double)((2 << 8) - 1) / ((2 << colorDepth) - 1);
	return (unsigned char)(r * floor((double)p / (2 << (8 - colorDepth))));
}

void ocvCuantify(IplImage *src, IplImage *dst, unsigned int colorDepth) {
	colorDepth = (colorDepth > 8) ? 8 : colorDepth;

	for (unsigned int y = 0; y < src->height; y++) {
		for (unsigned int x = 0; x < src->width; x++) {
			CvScalar p = cvGet2D(src, y, x);
			CvScalar q = cvScalar(
				  cuantifyEquation(p.val[0], colorDepth)
				, cuantifyEquation(p.val[1], colorDepth)
				, cuantifyEquation(p.val[2], colorDepth)
				, p.val[3]);
			cvSet2D(dst, y, x, q);
		}
	}
}
