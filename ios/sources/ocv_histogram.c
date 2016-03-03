#include "messages.h"
#include "ocv_histogram.h"

#include <string.h>

DIP_EXTERN double *data_histogram(double frequencies[256], unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep);

size_t calcularHistograma(IplImage *src, size_t *binsCount, size_t **bins);
void graficarHistograma(IplImage *dst, size_t binsCount, size_t *bins);

IplImage *ocv_histogram1(IplImage *image) {
	if (!image) { present(1, "!image"); return NULL; }

	unsigned char *src = (unsigned char *)image->imageData;
	unsigned int width = image->width;
	unsigned int height = image->height;
	unsigned int widthStep = image->widthStep;

	double frequencies[256];
	data_histogram(frequencies, src, width, height, widthStep);

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

IplImage *ocv_histogram(IplImage *image) {
	if (!image) { present(1, "!image"); return NULL; }
#if 1
	size_t binsCount = 0;
	size_t *bins = NULL;
	calcularHistograma(image, &binsCount, &bins);
	IplImage *subimage = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);
	graficarHistograma(subimage, binsCount, bins);
	cvReleaseImage(&subimage); //return subimage;
#endif
	IplImage *iplImageOut = NULL;
	if (image->nChannels == 1) {
		iplImageOut = ocv_histogram1(image);
	} else if (image->nChannels == 3) {
		IplImage *iplImgIR = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		IplImage *iplImgIG = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		IplImage *iplImgIB = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
		cvSplit(image, iplImgIR, iplImgIG, iplImgIB, NULL);

		IplImage *iplImgOR = ocv_histogram1(iplImgIR);
		IplImage *iplImgOG = ocv_histogram1(iplImgIG);
		IplImage *iplImgOB = ocv_histogram1(iplImgIB);
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

double *data_histogram(double frequencies[256], unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep) {

	if (!src) {
		present(1, "!src");
		return NULL;
	}

	unsigned long long freqs[256];
	memset(freqs, 0, sizeof(freqs));

	int maxFreq = 0;
	for(int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int f = src[i + j * widthStep];
			freqs[f]++;
			if (freqs[f] >= freqs[maxFreq]) {
				maxFreq = f;
			}
		}
	}

	for (int i = 0; i < 256; i++) {
		frequencies[i] = (double)freqs[i] / freqs[maxFreq];
	}

	return frequencies;
}

#include <syslog.h>

#define SL syslog(LOG_WARNING, "%s::%d", __PRETTY_FUNCTION__, __LINE__);

size_t calcularHistograma(IplImage *src, size_t *binsCount, size_t **bins) {
	if (src == NULL || bins == NULL) {
		return -1;
	}
	static int _trueChannels = 1;
	static int _trueChannel = 1;
	int channels = src->nChannels;
	int hist_size = 256;
	if (*bins == NULL) {
		*bins = (size_t *)calloc(channels * hist_size, sizeof(size_t));
		*binsCount = channels;
	}
	//if (channels == 3) printf("%d (%p)", *binsCount, *bins);

	//Actuo en funcion la cantidad de colores de la imagen
	if (channels == 1) {
		float range[] = { 0, 256 };
		float* ranges[] = { range };

		CvHistogram *hist_bw = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
		cvCalcHist(&src, hist_bw, 0, NULL);

		for (int i = 0; i < hist_size; i++) {
			(*bins)[_trueChannel * hist_size + i] = cvRound(cvGetReal1D(hist_bw->bins, i));
		}

		cvReleaseHist(&hist_bw);
	} else if (src->nChannels == 3) {
		_trueChannels = 3;
		IplImage *channelA = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
		IplImage *channelB = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
		IplImage *channelC = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
		cvSplit(src, channelA, channelB, channelC, NULL);

		_trueChannel = 0;
		calcularHistograma(channelA, binsCount, bins);
		cvReleaseImage(&channelA);

		_trueChannel = 1;
		calcularHistograma(channelB, binsCount, bins);
		cvReleaseImage(&channelB);

		_trueChannel = 2;
		calcularHistograma(channelC, binsCount, bins);
		cvReleaseImage(&channelC);

		_trueChannels = 1;
		_trueChannel = 1;
	} else {
		return -1;
	}
	return 0;
}

void graficarHistograma(IplImage *dst, size_t binsCount, size_t *bins) {
	static CvScalar hist_color;
	static char ini = 0;
	if (!ini) {
		ini = 1;
		hist_color = cvScalarAll(255);
	}
	static int _trueChannels = 1;
	static int _trueChannel = 1;
	int hist_size = 256;

	//cvSet(ImagenHistorial, cvScalarAll(0), 0);

	//Actuo en funcion la cantidad de colores de la imagen
	if (dst->nChannels == 1) {
		size_t max_value = 0;
		size_t _bins[256];
		for (size_t i = 0; i < hist_size; i++) {
			_bins[i] = bins[_trueChannel * hist_size + i];
			max_value = (_bins[i] > max_value) ? _bins[i] : max_value;
		}
		for (size_t i = 0; i < hist_size; i++) {
			_bins[i] /= max_value;
		}
		float w_scale = ((float)dst->width) / hist_size;

		//Graficar en la imagen
		for (int i = 0; i < hist_size; i++) {
			cvLine(dst,
				cvPoint((int)(i       * w_scale), dst->height - _bins[i]),
				cvPoint((int)((i + 1) * w_scale), dst->height - _bins[i]),
				hist_color, 2, 8, 0);
		}

	} else if (dst->nChannels == 3) {
		_trueChannels = 3;
		IplImage *channelA = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);
		IplImage *channelB = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);
		IplImage *channelC = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);
		cvSplit(dst, channelA, channelB, channelC, NULL);

		_trueChannel = 0;
		hist_color = cvScalar(255, 0, 0, 0);
		graficarHistograma(channelA, binsCount, bins);

		_trueChannel = 1;
		hist_color = cvScalar(0, 255, 0, 0);
		graficarHistograma(channelB, binsCount, bins);

		_trueChannel = 2;
		hist_color = cvScalar(0, 0, 255, 0);
		graficarHistograma(channelC, binsCount, bins);

		_trueChannels = 1;
		_trueChannel = 1;

		hist_color = cvScalarAll(255);

		cvMerge(channelA, channelB, channelC, NULL, dst);

		cvReleaseImage(&channelA);
		cvReleaseImage(&channelB);
		cvReleaseImage(&channelC);
	}
}
