#include "messages.h"
#include "ocv_histogramac.h"

#include <stdlib.h>
#include <opencv2/imgproc/imgproc_c.h>

DIP_EXTERN double *data_histogramac(double frequencies[256], unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep);

IplImage *ocv_histogramac1(IplImage *image) {
	if (!image) { present(1, "!image"); return NULL; }

	unsigned char *src = (unsigned char *)image->imageData;
	unsigned int width = image->width;
	unsigned int height = image->height;
	unsigned int widthStep = image->widthStep;

	double frequencies[256];
	data_histogramac(frequencies, src, width, height, widthStep);

	IplImage *image2 = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
	cvSet(image2, cvScalarAll(0), NULL);

	double spacing = (double)width / 256;
	for (int i = 0; i < 255; i++) {
		cvLine(image2,
			cvPoint((int)(i *	spacing), height * (1 - frequencies[i])),
			cvPoint((int)((i+1)*spacing), height * (1 - frequencies[i])),
			cvScalarAll(255),
			1,8,0);
	}

	return image2;
}

IplImage *ocv_histogramac(IplImage *image) {
	if (!image) { present(1, "!image"); return NULL; }

	IplImage *iplImageOut = NULL;
	if (image->nChannels == 1) {
		iplImageOut = ocv_histogramac1(image);
	} else if (image->nChannels == 3) {
		IplImage *iplImgIR = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		IplImage *iplImgIG = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		IplImage *iplImgIB = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		cvSplit(image, iplImgIR, iplImgIG, iplImgIB, NULL);

		IplImage *iplImgOR = ocv_histogramac1(iplImgIR);
		IplImage *iplImgOG = ocv_histogramac1(iplImgIG);
		IplImage *iplImgOB = ocv_histogramac1(iplImgIB);
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

DIP_EXTERN double *data_histogram(double frequencies[256], unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep);

double *data_histogramac(double frequencies[256], unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep) {

	if (!src) {
		present(1, "!src");
		return NULL;
	}

	double freqs[256];
	data_histogram(freqs, src, width, height, widthStep);

	frequencies[0] = freqs[0];
	for (int i = 1; i < 256; i++) {
		frequencies[i] = freqs[i] + frequencies[i - 1];
	}

	for (int i = 1; i < 256; i++) {
		frequencies[i] /= frequencies[255];
	}

	return frequencies;
}
