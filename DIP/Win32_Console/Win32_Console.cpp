// Win32_Console.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/core/core_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <iostream>

#define TRY(X) { char $E = 0; try { X } catch (cv::Exception *e) { std::cout << e->what(); $E = 1; } $E; }

#define INPUT_WINDOW	"Input Window"
#define OUTPUT_WINDOW	"Output Window"
#define CONTROL_WINDOW	"Control Window"

IplImage *BLACK1D = NULL;

typedef struct Userdata {
	int key;

	int value;
	int kernelSize;

	int lowThreshold;
	int highThreshold;

	CvScalar pixel;

	int minScalar0;
	int minScalar1;
	int minScalar2;
	int maxScalar0;
	int maxScalar1;
	int maxScalar2;

	IplImage *input[2];
	IplImage *output[2];
} Userdata;

char operateImage(Userdata *userdata);

void mouseCallback(int event, int x, int y, int flags, void* userdata);
void trackbarCallback(int val);

int _tmain(int argc, char *argv[]) {
	Userdata userdata;
	memset(&userdata, 0, sizeof(Userdata));
	userdata.kernelSize = 5;
	userdata.lowThreshold = 70;
	userdata.highThreshold = 200;
	userdata.minScalar0 = 100;
	userdata.minScalar1 = 0;
	userdata.minScalar2 = 66;
	userdata.maxScalar0 = 120;
	userdata.maxScalar1 = 20;
	userdata.maxScalar2 = 210;

	CvCapture *cv_cap = cvCaptureFromCAM(CV_CAP_ANY);
	if (!cv_cap) {
		std::cout << "Could not open camera\n";
		cvWaitKey(0);
		return -1;
	}

	int cam_width = (int)cvGetCaptureProperty(cv_cap, CV_CAP_PROP_FRAME_WIDTH);
	int cam_height = (int)cvGetCaptureProperty(cv_cap, CV_CAP_PROP_FRAME_HEIGHT);
	CvSize cam_size = cvSize(cam_width, cam_height);

	BLACK1D = cvCreateImage(cam_size, IPL_DEPTH_8U, 1);
	cvSet(BLACK1D, cvScalarAll(0), NULL);

	userdata.input[0] = cvCreateImage(cam_size, IPL_DEPTH_8U, 3);
	userdata.input[1] = cvCreateImage(cam_size, IPL_DEPTH_8U, 3);
	cvNamedWindow(INPUT_WINDOW, 0);
	cvResizeWindow(INPUT_WINDOW, cam_width, cam_height);
	cvSetMouseCallback(INPUT_WINDOW, mouseCallback, &userdata);

	userdata.output[0] = cvCreateImage(cam_size, IPL_DEPTH_8U, 3);
	userdata.output[1] = cvCreateImage(cam_size, IPL_DEPTH_8U, 3);
	cvNamedWindow(OUTPUT_WINDOW, 0);
	cvResizeWindow(OUTPUT_WINDOW, cam_width, cam_height);

#define SHOW_CONTROLS 1
#if SHOW_CONTROLS
	//	IplImage *control = cvCreateImage(cvSize(320, 60), IPL_DEPTH_8U, 3);
	cvNamedWindow(CONTROL_WINDOW, 0);
	cvResizeWindow(CONTROL_WINDOW, 800, 80);
	//cvCreateTrackbar("kernel size", CONTROL_WINDOW, &(userdata.kernelSize), 15, trackbarCallback);

	cvCreateTrackbar("min 0", CONTROL_WINDOW, &(userdata.minScalar0), 255, trackbarCallback);
	cvCreateTrackbar("min 1", CONTROL_WINDOW, &(userdata.minScalar1), 255, trackbarCallback);
	cvCreateTrackbar("min 2", CONTROL_WINDOW, &(userdata.minScalar2), 255, trackbarCallback);
	cvCreateTrackbar("max 0", CONTROL_WINDOW, &(userdata.maxScalar0), 255, trackbarCallback);
	cvCreateTrackbar("max 1", CONTROL_WINDOW, &(userdata.maxScalar1), 255, trackbarCallback);
	cvCreateTrackbar("max 2", CONTROL_WINDOW, &(userdata.maxScalar2), 255, trackbarCallback);

	//cvCreateTrackbar("value", CONTROL_WINDOW, &(userdata.value), (int)sqrt(cam_height * cam_width), trackbarCallback);
#endif

	int use_cam = 1;
	IplImage *input = userdata.input[0];
	IplImage *output = userdata.output[0];

	while ((userdata.key = cvWaitKey(50)) != 27) { // wait 50 ms (20 FPS) or for ESC key
		IplImage *cam = cvQueryFrame(cv_cap); // get frame
		if (!cam) {
			std::cout << "no input\n";
			continue;
		}

		switch (userdata.key) {
		case ' ':
			use_cam = !use_cam;
			break;
		}

		if (!use_cam) {
			operateImage(&userdata);
		}
		else {
			cvCopy(cam, input, NULL);
			//cvResizeWindow(INPUT_WINDOW, input->width, input->height);
			cvShowImage(INPUT_WINDOW, input);

			operateImage(&userdata);
		}
		//cvResizeWindow(OUTPUT_WINDOW, output->width, output->height);
		cvShowImage(OUTPUT_WINDOW, output);
	}
	/* clean up */
	cvReleaseCapture(&cv_cap);

	cvReleaseImage(&userdata.input[0]);
	cvReleaseImage(&userdata.input[1]);
	cvReleaseImage(&userdata.output[0]);
	cvReleaseImage(&userdata.output[1]);

	cvDestroyWindow(INPUT_WINDOW);
	cvDestroyWindow(OUTPUT_WINDOW);
#if SHOW_CONTROLS
	cvDestroyWindow(CONTROL_WINDOW);
#endif
	return 0;
}

char PointInRect(int x, int y, int width, int height) {
	return !(y < 0 || y >= height || x < 0 || x >= width);
}


int filterByVolume(IplImage *src, IplImage *dst, long minVolume) {
	int width = src->width;
	int height = src->height;

	static int *data = NULL;
	if (!data) {
		data = (int *)malloc(height * width * sizeof(int));
	}
	memset(data, 0, height * width * sizeof(int));

	int n = 1;
	char p_val = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			CvScalar px = cvGet2D(src, y, x);
			char val = (px.val[0] == 255);

			if (val == 0) {
				//if (PointInRect(x, y, width, height))
				data[y * width + x] = n;
			}
			else {
				data[y * width + x] = 0;
				if (val != p_val) {
					n++;
				}
			}
			p_val = val;
		}
		n++;
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int d = data[y * width + x];
			if (d == 0) {
				cvSet2D(dst, y, x, cvScalar(255, 0, 0));
			}
			else {
				char a = (int)((float)d / n);
				cvSet2D(dst, y, x, cvScalar(a, 0, 0));
			}
		}
	}

	return 0;
}

int filterByHSV(IplImage *src, CvScalar minHSV, CvScalar maxHSV, IplImage *dst) {
	cvCopy(src, dst, NULL);

	cvSmooth(dst, dst, CV_GAUSSIAN, 13, 0, 0, 0);

	IplImage *tmp1dH = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dS = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dV = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);

	cvSplit(dst, tmp1dH, tmp1dS, tmp1dV, NULL);

	cvInRangeS(tmp1dH, cvScalar(minHSV.val[0], 0, 0), cvScalar(maxHSV.val[0], 0, 0), tmp1dH);
	cvInRangeS(tmp1dS, cvScalar(minHSV.val[1], 0, 0), cvScalar(maxHSV.val[0], 0, 0), tmp1dS);
	cvInRangeS(tmp1dV, cvScalar(minHSV.val[2], 0, 0), cvScalar(maxHSV.val[2], 0, 0), tmp1dV);

	cvMerge(tmp1dH, tmp1dS, tmp1dV, NULL, dst);

	cvReleaseImage(&tmp1dH);
	cvReleaseImage(&tmp1dS);
	cvReleaseImage(&tmp1dV);

	cvCvtColor(dst, dst, CV_HSV2RGB);

	IplImage *tmp1d = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(dst, tmp1d, CV_BGR2GRAY);
	cvMerge(tmp1d, tmp1d, tmp1d, NULL, dst);
	cvReleaseImage(&tmp1d);

	return 0;
}

void GraficarHistograma(IplImage *Imagen, IplImage *ImagenHistorial)
{
	CvHistogram *hist_red, *hist_green, *hist_blue, *hist_bw;

	IplImage *channel = cvCreateImage(cvGetSize(Imagen), IPL_DEPTH_8U, 1);

	int i;

	int hist_size = 256;
	float range[] = { 0, 256 };
	float* ranges[] = { range };

	float max_value = 0.0;
	float max = 0.0;
	float w_scale = 0.0;

	cvSet(ImagenHistorial, cvScalarAll(0), 0);

	//Actúo en función de la cantidad de colores de la imágen
	if (Imagen->nChannels == 3)
	{
		// Crear arrays unidimensionales para los histogramas
		hist_red = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
		hist_green = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
		hist_blue = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);

		//Separo los colores y obtengo cada histograma

		//Empiezo por el color rojo
		cvSetImageCOI(Imagen, 3);
		cvCopy(Imagen, channel, NULL);
		cvResetImageROI(Imagen);

		//Obtengo los datos del histograma rojo
		cvCalcHist(&channel, hist_red, 0, NULL);

		//Repito con el color verde
		cvSetImageCOI(Imagen, 2);
		cvCopy(Imagen, channel, NULL);
		cvResetImageROI(Imagen);

		cvCalcHist(&channel, hist_green, 0, NULL);

		//Misma operación para el color azul
		cvSetImageCOI(Imagen, 1);
		cvCopy(Imagen, channel, NULL);
		cvResetImageROI(Imagen);

		cvCalcHist(&channel, hist_blue, 0, NULL);

		//Obtengo el máximo de cada histograma para las gráficas
		cvGetMinMaxHistValue(hist_red, 0, &max_value, 0, 0);

		cvGetMinMaxHistValue(hist_green, 0, &max, 0, 0);

		if (max > max_value)max_value = max;

		cvGetMinMaxHistValue(hist_blue, 0, &max, 0, 0);

		if (max > max_value)max_value = max;

		//cvConvertScale redefine a cvScale
		cvScale(hist_red->bins, hist_red->bins, ((float)ImagenHistorial->height) / max_value, 0);
		cvScale(hist_green->bins, hist_green->bins, ((float)ImagenHistorial->height) / max_value, 0);
		cvScale(hist_blue->bins, hist_blue->bins, ((float)ImagenHistorial->height) / max_value, 0);

		printf("Scale: %4.2f pixels per 100 units\n", max_value * 100 / ((float)ImagenHistorial->height));
		//A scale to estimate the number of pixels

		//Calcular el ancho de las barras / el punto medio de los pixels
		w_scale = ((float)ImagenHistorial->width) / hist_size;

		// Graficar el histograma
		/*for(i = 0; i < hist_size; i++ )	//Grafico con rectángulos
		{
		cvRectangle( ImagenHistorial, cvPoint((int)i*w_scale , ImagenHistorial->height),
		cvPoint((int)(i+1)*w_scale, ImagenHistorial->height - cvRound(cvGetReal1D(hist_red->bins,i))),
		CV_RGB(255,0,0), -1, 8, 0 );
		cvRectangle( ImagenHistorial, cvPoint((int)i*w_scale , ImagenHistorial->height),\
		cvPoint((int)(i+1)*w_scale, ImagenHistorial->height - cvRound(cvGetReal1D(hist_green->bins,i))),\
		CV_RGB(0,255,0), -1, 8, 0 );
		cvRectangle( ImagenHistorial, cvPoint((int)i*w_scale , ImagenHistorial->height),
		cvPoint((int)(i+1)*w_scale, ImagenHistorial->height - cvRound(cvGetReal1D(hist_blue->bins,i))),
		CV_RGB(0,0,255), -1, 8, 0 );
		}*/

		for (i = 1; i < hist_size; i++)	//Grafico con líneas
		{
			cvLine(ImagenHistorial, cvPoint((int)(i*w_scale), ImagenHistorial->height - cvRound(cvGetReal1D(hist_red->bins, i - 1))), \
				cvPoint((int)((i + 1)*w_scale), ImagenHistorial->height - cvRound(cvGetReal1D(hist_red->bins, i))), \
				CV_RGB(255, 0, 0), 4, 8, 0);
			cvLine(ImagenHistorial, cvPoint((int)(i*w_scale), ImagenHistorial->height - cvRound(cvGetReal1D(hist_green->bins, i - 1))), \
				cvPoint((int)((i + 1)*w_scale), ImagenHistorial->height - cvRound(cvGetReal1D(hist_green->bins, i))), \
				CV_RGB(0, 255, 0), 4, 8, 0);
			cvLine(ImagenHistorial, cvPoint((int)(i*w_scale), ImagenHistorial->height - cvRound(cvGetReal1D(hist_blue->bins, i - 1))), \
				cvPoint((int)((i + 1)*w_scale), ImagenHistorial->height - cvRound(cvGetReal1D(hist_blue->bins, i))), \
				CV_RGB(0, 0, 255), 4, 8, 0);
		}

		cvReleaseHist(&hist_red);
		cvReleaseHist(&hist_green);
		cvReleaseHist(&hist_blue);
	}

	else {

		hist_bw = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);

		cvCalcHist(&Imagen, hist_bw, 0, NULL);
		cvGetMinMaxHistValue(hist_bw, 0, &max_value, 0, 0);

		cvScale(hist_bw->bins, hist_bw->bins, ((float)ImagenHistorial->height) / max_value, 0);

		printf("Scale bw: %4.2f pixels per 100 units\n", max_value * 100 / ((float)ImagenHistorial->height));

		w_scale = ((float)ImagenHistorial->width) / hist_size;

		cvSet(ImagenHistorial, cvScalarAll(0), 0);

		//Graficar en la imagen
		/*for(i = 0; i < hist_size; i++ )
		{
		cvRectangle( ImagenHistorial, cvPoint((int)i*w_scale , ImagenHistorial->height),
		cvPoint((int)(i+1)*w_scale, ImagenHistorial->height - cvRound(cvGetReal1D(hist_bw->bins,i))),
		CV_RGB(0,0,0), -1, 8, 0 );
		}*/

		for (i = 1; i < hist_size; i++)
		{
			cvLine(ImagenHistorial, cvPoint((int)(i*w_scale), ImagenHistorial->height - cvRound(cvGetReal1D(hist_bw->bins, i - 1))),
				cvPoint((int)((i + 1)*w_scale), ImagenHistorial->height - cvRound(cvGetReal1D(hist_bw->bins, i))),
				CV_RGB(0, 0, 0), 2, 8, 0);
		}

		cvReleaseHist(&hist_bw);
	}

}


char operateImage(Userdata *userdata) {
	if (!userdata) {
		return 0;
	}

	IplImage *image1 = userdata->input[0];
	IplImage *image2 = userdata->input[1];
	IplImage *imageOut = userdata->output[0];
	IplImage *imageOut2 = userdata->output[1];

	static int color_mode = 0;
	static int smooth_mode = 0;
	static int otsu_mode = 0;
	static int close_mode = 0;
	static int canny_mode = 0;
	static int filter_mode = 0;
	static int contour_mode = 0;
	static int hsv_mode = 0;

	int key = userdata->key;
	switch (key) {
	case 'g':
		color_mode++;
		color_mode %= 5;
		break;
	case 's':
		smooth_mode = !smooth_mode;
		break;
	case 'o':
		otsu_mode = !otsu_mode;
		break;
	case 'e':
		close_mode = !close_mode;
		break;
	case 'c':
		canny_mode = !canny_mode;
		break;
	case 'b':
		contour_mode = !contour_mode;
		break;
	case 'f':
		filter_mode = !filter_mode;
		break;
	case 'h':
		hsv_mode = !hsv_mode;
		break;
	default:
		//cout << key << "\n";
		break;
	}

	int value = userdata->value;
	int kernelSize = userdata->kernelSize;
	kernelSize += 1 - (kernelSize % 2);
	int lowThreshold = userdata->lowThreshold;
	int highThreshold = userdata->highThreshold;
	CvScalar minScalar = cvScalar(userdata->minScalar0, userdata->minScalar1, userdata->minScalar2);
	CvScalar maxScalar = cvScalar(userdata->maxScalar0, userdata->maxScalar1, userdata->maxScalar2);

	static IplImage *tmp1d = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 1);
	static IplImage *tmp3d = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 3);
	static IplImage *tmp3d2 = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 3);

	static IplImage *backgroundAcc = cvCreateImage(cvGetSize(image1), IPL_DEPTH_32F, 3);
	static IplImage *background = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 3);

	std::cout << "                                                                         \r";

	char img_full_channel = 0;
	switch (color_mode) {
	case 0:
		std::cout << "Gray";
		cvCvtColor(image1, tmp1d, CV_BGR2GRAY);
		break;
	case 1: // Hue mode
		std::cout << "Hue";
		cvCvtColor(image1, tmp3d, CV_BGR2HSV);
		cvSplit(tmp3d, tmp1d, NULL, NULL, NULL);
		break;
	case 2: // Saturation mode
		std::cout << "Saturation";
		cvCvtColor(image1, tmp3d, CV_BGR2HSV);
		cvSplit(tmp3d, NULL, tmp1d, NULL, NULL);
		break;
	case 3: // Brightness mode
		std::cout << "Brightness";
		cvCvtColor(image1, tmp3d, CV_BGR2HSV);
		cvSplit(tmp3d, NULL, NULL, tmp1d, NULL);
		break;
	case 4: // 
		std::cout << "Color";
		img_full_channel = 1;
		break;
	}

	if (img_full_channel) { // Image has 3 channel
#if 1
		//		GraficarHistograma(image1, tmp3d);
		filterByHSV(image1, minScalar, maxScalar, tmp3d);
#else
		cvCvtColor(image1, tmp3d, CV_BGR2HSV);

		IplImage *tmp1dH = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 1);
		IplImage *tmp1dS = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 1);
		IplImage *tmp1dV = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 1);

		cvSmooth(tmp3d, tmp3d, CV_GAUSSIAN, 13, 0, 0, 0);
		cvSplit(tmp3d, tmp1dH, tmp1dS, tmp1dV, NULL);
		cvInRangeS(tmp1dH, cvScalar(minScalar.val[0], 0, 0), cvScalar(maxScalar.val[0], 0, 0), tmp1dH);
		cvInRangeS(tmp1dS, cvScalar(minScalar.val[1], 0, 0), cvScalar(maxScalar.val[1], 0, 0), tmp1dS);
		cvInRangeS(tmp1dV, cvScalar(minScalar.val[2], 0, 0), cvScalar(maxScalar.val[2], 0, 0), tmp1dV);
		cvMerge(tmp1dH, tmp1dS, tmp1dV, NULL, tmp3d);

		cvReleaseImage(&tmp1dH);
		cvReleaseImage(&tmp1dS);
		cvReleaseImage(&tmp1dV);

		cvCvtColor(tmp3d, tmp3d, CV_HSV2RGB);
		cvCvtColor(tmp3d, tmp1d, CV_BGR2GRAY);
		cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
#endif

	}
	else { // Image has 1 channel

		cvSmooth(tmp1d, tmp1d, CV_GAUSSIAN, 5, 0, 0, 0);

		if (otsu_mode) { // Apply Otsu's method
			std::cout << ", Otsu";
			cvThreshold(tmp1d, tmp1d, 0, 255, CV_THRESH_OTSU);
		}

		if (smooth_mode) { // Apply Gaussian smoothing
			std::cout << ", Gauss";
			cvSmooth(tmp1d, tmp1d, CV_GAUSSIAN, 5, 0, 0, 0);
		}

		if (close_mode) {
			std::cout << ", closE";
			int n = kernelSize;
			cvErode(tmp1d, tmp1d, NULL, n);
			cvDilate(tmp1d, tmp1d, NULL, n);
		}

		if (canny_mode) { // Apply Canny's method
			std::cout << ", Canny";
			cvCanny(tmp1d, tmp1d, lowThreshold, highThreshold, 3);
			cvDilate(tmp1d, tmp1d, NULL, 1);
			cvErode(tmp1d, tmp1d, NULL, 1);
		}

		if (filter_mode) {
			std::cout << ", Filter";
			filterByVolume(tmp1d, tmp1d, value);
		}

		cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);

		if (contour_mode) {
			std::cout << ", contours(b)";
			CvMemStorage *storage = cvCreateMemStorage(0);
			CvSeq *contours = NULL;
			int n = cvFindContours(tmp1d, storage, &contours, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
			//std::cout << ", (" << n <<","<< contours->total <<")contours";
			for (int i = 0; contours != NULL; contours = contours->h_next, i++) {
				int cc = (int)((float)(255 * i) / contours->total);
				CvScalar colorpx = CV_RGB((cc) % 256, (cc + 256 / 3) % 256, (cc + 256 * 2 / 3) % 256);
				cvDrawContours(tmp3d, contours, colorpx, CV_RGB(0, 0, 0), -1, CV_FILLED, 8, cvPoint(0, 0));
			}
		}

	}

	std::cout << "\r";

	cvCopy(image1, image2, NULL);
	cvCopy(imageOut, imageOut2, NULL);
	cvCopy(tmp3d, imageOut, NULL);

	//cvReleaseImage(&tmp1d);
	//cvReleaseImage(&tmp3d);
	//cvReleaseImage(&tmp3d2);

	return 0;
}


void trackbarCallback(int val) {
}

void mouseCallback(int event, int x, int y, int flags, void *_userdata) {
	//std::cout << "Mouse event (" << event << ") with flag (" << flags << ") at (" << x << ", " << y << ")";
	Userdata *userdata = (Userdata *)_userdata;
	if (!userdata) {
		return;
	}

	IplImage *image = userdata->input[0];

	static char down = 0;
	switch (event) {
	case CV_EVENT_LBUTTONDOWN:
	case CV_EVENT_RBUTTONDOWN:
	case CV_EVENT_MBUTTONDOWN:
		down = 1;
		break;
	case CV_EVENT_LBUTTONUP:
	case CV_EVENT_RBUTTONUP:
	case CV_EVENT_MBUTTONUP:
		down = 0;
		break;
	default:
		break;
	}

	static IplImage *tmp1d = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
	static IplImage *tmp3d = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);

	switch (event) {
	case CV_EVENT_LBUTTONDOWN: {
		if (image) {
			userdata->pixel = cvGet2D(image, y, x);
			CvScalar px = userdata->pixel;
			std::cout << "?RGB:" << px.val[0] << "," << px.val[1] << "," << px.val[2];
			cvCvtColor(image, tmp3d, CV_BGR2HSV);
			px = cvGet2D(tmp3d, y, x);
			std::cout << "?HSV:" << px.val[0] << "," << px.val[1] << "," << px.val[2];
		}
	}; break;
	case CV_EVENT_RBUTTONDOWN: {
	}; break;
	case CV_EVENT_MBUTTONDOWN: {
	}; break;
	case CV_EVENT_MOUSEMOVE: {
	}; break;
	default: {
	}; break;
	}

	//std::cout << "\r";
}