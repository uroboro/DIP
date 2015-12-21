#include "messages.h"
#include "ocv_cuantify.h"

DIP_EXTERN unsigned char *data_cuantify(unsigned char **dst, unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep,
	unsigned int colorDepth);

IplImage *ocv_cuantify1(IplImage *image, unsigned int colorDepth) {
	if (!image) { present(1, "!image"); return NULL; }

	unsigned char *src = (unsigned char *)image->imageData;
	unsigned int width = image->width;
	unsigned int height = image->height;
	unsigned int widthStep = image->widthStep;

	IplImage *image2 = cvCloneImage(image);
	unsigned char *dst = (unsigned char *)image2->imageData;

	data_cuantify(&dst, src, width, height, widthStep, colorDepth);

	return image2;
}

IplImage *ocv_cuantify(IplImage *image, unsigned int colorDepth) {
	if (!image) { present(1, "!image"); return NULL; }

	IplImage *iplImageOut = NULL;
	if (image->nChannels == 1) {
		iplImageOut = ocv_cuantify1(image, colorDepth);
	} else if (image->nChannels == 3) {
		IplImage *iplImgIR = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		IplImage *iplImgIG = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		IplImage *iplImgIB = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		cvSplit(image, iplImgIR, iplImgIG, iplImgIB, NULL);

		IplImage *iplImgOR = ocv_cuantify1(iplImgIR, colorDepth);
		IplImage *iplImgOG = ocv_cuantify1(iplImgIG, colorDepth);
		IplImage *iplImgOB = ocv_cuantify1(iplImgIB, colorDepth);
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
#include <math.h>
#include "data_filter.h"

unsigned char cuantifyEquation(unsigned char p, unsigned int colorDepth);

unsigned char *data_cuantify(unsigned char **dst, unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep,
	unsigned int colorDepth) {

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

	colorDepth = CLAMP(colorDepth, 0, 8);

	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			dest[y * widthStep2 + x] = cuantifyEquation(src[y * widthStep + x], colorDepth);
		}
	}

	return dest;
}

unsigned char cuantifyEquation(unsigned char p, unsigned int colorDepth) {
	double r = (double)((2 << 8) - 1) / ((2 << colorDepth) - 1);
	return (unsigned char)(r * floor((double)p / (2 << (8 - colorDepth))));
}
