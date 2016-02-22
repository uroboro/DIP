#include "ocv_samplify.h"

void ocvSamplify(IplImage *src, IplImage *dst, unsigned int sampleSize) {
	unsigned int smallerDimension = (src->width < src->height) ? src->width : src->height;
	sampleSize = (sampleSize > smallerDimension) ? smallerDimension : sampleSize;

	for (unsigned int y = 0; y < src->height; y += sampleSize) {
		for (unsigned int x = 0; x < src->width; x += sampleSize) {
			CvScalar sum = cvScalarAll(0);
			for (unsigned int j = 0; j < sampleSize; j++) {
				for (unsigned int i = 0; i < sampleSize; i++) {
					if (x + i >= src->width || y + j >= src->height) continue;
					CvScalar p = cvGet2D(src, y, x);
					for (unsigned int c = 0; c < 4; c++) {
						sum.val[c] += p.val[c];
					}
				}
			}
			for (unsigned int c = 0; c < 4; c++) {
				sum.val[c] /= sampleSize * sampleSize;
			}
			for (unsigned int j = 0; j < sampleSize; j++) {
				for (unsigned int i = 0; i < sampleSize; i++) {
					if (x + i >= src->width || y + j >= src->height) continue;
					cvSet2D(dst, y + j, x + i, sum);
				}
			}
		}
	}
}
