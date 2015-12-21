#include "messages.h"
#include "ocv_samplify.h"

DIP_EXTERN unsigned char *data_samplify(unsigned char **dst, unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep,
	unsigned int sampleSize);

IplImage *ocv_samplify1(IplImage *image, unsigned int sampleSize) {
	if (!image) { present(1, "!image"); return NULL; }

	unsigned char *src = (unsigned char *)image->imageData;
	unsigned int width = image->width;
	unsigned int height = image->height;
	unsigned int widthStep = image->widthStep;

	IplImage *image2 = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
	unsigned char *dst = (unsigned char *)image2->imageData;

	data_samplify(&dst, src, width, height, widthStep, sampleSize);

	return image2;
}

IplImage *ocv_samplify(IplImage *image, unsigned int sampleSize) {
	if (!image) { present(1, "!image"); return NULL; }

	IplImage *iplImageOut = NULL;
	if (image->nChannels == 1) {
		iplImageOut = ocv_samplify1(image, sampleSize);
	} else if (image->nChannels == 3) {
		IplImage *iplImgIR = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		IplImage *iplImgIG = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		IplImage *iplImgIB = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		cvSplit(image, iplImgIR, iplImgIG, iplImgIB, NULL);

		IplImage *iplImgOR = ocv_samplify1(iplImgIR, sampleSize);
		IplImage *iplImgOG = ocv_samplify1(iplImgIG, sampleSize);
		IplImage *iplImgOB = ocv_samplify1(iplImgIB, sampleSize);
		cvReleaseImage(&iplImgIR);
		cvReleaseImage(&iplImgIG);
		cvReleaseImage(&iplImgIB);

		iplImageOut = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);
		cvMerge(iplImgOR, iplImgOG, iplImgOB, NULL, iplImageOut);
		cvReleaseImage(&iplImgOR);
		cvReleaseImage(&iplImgOG);
		cvReleaseImage(&iplImgOB);
	}
	return iplImageOut;
}

#include <stdlib.h>

#include "data_filter.h"

unsigned char *data_samplify(unsigned char **dst, unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep,
	unsigned int sampleSize) {

		if (!src) {
		present(1, "!src");
		return NULL;
	}

	unsigned char *dest = NULL;
	unsigned int widthStep2 = widthStep;
		if (dst == NULL || *dst == NULL) {
			dest = calloc(width * height, sizeof(unsigned char));
		if (!dest) {
			return NULL;
		}

		if (dst != NULL) {
				*dst = dest;
		}

		widthStep2 = width;
	} else {
			dest = *dst;
	}

		sampleSize = CLAMP(sampleSize, 0, (width < height) ? width : height);

	for (unsigned int y = 0; y < height; y += sampleSize) {
		for (unsigned int x = 0; x < width; x += sampleSize) {
				unsigned int sum = 0;
			for (unsigned int j = 0; j < sampleSize; j++) {
				for (unsigned int i = 0; i < sampleSize; i++) {
					if (x + i >= width || y + j >= height) continue;
					sum += src[(y + j) * widthStep + (x + i)];
				}
			}
				sum /= sampleSize * sampleSize;
				for (unsigned int j = 0; j < sampleSize; j++) {
				for (unsigned int i = 0; i < sampleSize; i++) {
						if (x + i >= width || y + j >= height) continue;
						dest[(y + j) * widthStep + (x + i)] = sum;
				}
			}
		}
	}

	return dest;
}
