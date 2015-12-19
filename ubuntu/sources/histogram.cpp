#include "histogram.h"
#include <opencv2/imgproc/imgproc_c.h>

size_t calcularHistograma(IplImage *src, size_t *binsCount, size_t **bins) {
	if (src == NULL || bins == NULL) {
		return -1;
	}

	int channels = src->nChannels;
	int hist_size = 256;
	if (*bins == NULL) {
		*bins = (size_t*)calloc(channels * hist_size, sizeof(size_t));
		*binsCount = channels;
	}
	//if (channels == 3) printf("%d (%p)", *binsCount, *bins);

	//Actuo en funcion de la cantidad de colores de la imagen
	if (channels == 1) {
		float range[] = { 0, 256 };
		float* ranges[] = { range };

		CvHistogram *hist_bw = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
		cvCalcHist(&src, hist_bw, 0, NULL);

		for (int i = 1; i < hist_size; i++) {
			(*bins)[(*binsCount - 1) * hist_size + i] = cvRound(cvGetReal1D(hist_bw->bins, i));
		}

		cvReleaseHist(&hist_bw);
	} else if (src->nChannels == 3) {

		IplImage *channelA = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
		IplImage *channelB = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
		IplImage *channelC = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
		cvSplit(src, channelA, channelB, channelC, NULL);

		calcularHistograma(channelA, binsCount, bins);
		cvReleaseImage(&channelA);

		calcularHistograma(channelB, binsCount, bins);
		cvReleaseImage(&channelB);

		calcularHistograma(channelC, binsCount, bins);
		cvReleaseImage(&channelC);
	} else {
		return -1;
	}
	return 0;
}

void graficarHistograma(IplImage *dst, size_t binsCount, size_t *bins) {
	static CvScalar hist_color = cvScalarAll(255);
	size_t hist_size = 256;
	//cvSet(ImagenHistorial, cvScalarAll(0), 0);

	//Actuo en funcion de la cantidad de colores de la imagen
	if (dst->nChannels == 1) {
		size_t max_value = 0;
		size_t _bins[256];
		for (size_t i = 0; i < binsCount * hist_size; i++) {
			_bins[i] = bins[i];
			max_value = (_bins[i] > max_value) ? _bins[i] : max_value;
		}
		for (size_t i = 0; i < binsCount * hist_size; i++) {
			_bins[i] /= max_value;
		}
		float w_scale = ((float)dst->width) / hist_size;

		//Graficar en la imagen
		for (size_t i = 0; i < hist_size; i++) {
			cvLine(dst,
				cvPoint(binsCount * hist_size + (size_t)(i       * w_scale), dst->height - _bins[i]),
				cvPoint(binsCount * hist_size + (size_t)((i + 1) * w_scale), dst->height - _bins[i]),
				hist_color, 2, 8, 0);
		}

		//printf("Scale bw: %4.2f pixels per 100 units\r", max_value * 100 / ((float)ImagenHistorial->height));
	} else if (dst->nChannels == 3) {
		IplImage *channelA = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);
		IplImage *channelB = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);
		IplImage *channelC = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);
		cvSplit(dst, channelA, channelB, channelC, NULL);

		hist_color = cvScalar(255, 0, 0);
		graficarHistograma(channelA, binsCount, bins);

		hist_color = cvScalar(0, 255, 0);
		graficarHistograma(channelB, binsCount, bins);

		hist_color = cvScalar(0, 0, 255);
		graficarHistograma(channelC, binsCount, bins);

		hist_color = cvScalarAll(255);

		cvReleaseImage(&channelA);
		cvReleaseImage(&channelB);
		cvReleaseImage(&channelC);
	}
}
