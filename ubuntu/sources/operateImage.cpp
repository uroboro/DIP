#include "operateImage.h"

//#pragma mark - Userdata

Userdata getSessionUserdata(CvSize size) {
	Userdata userdata;
	memset(&userdata, 0, sizeof(Userdata));

	userdata.timestep = 50;

	userdata.kernelSize = 5;
	userdata.lowThreshold = 70;
	userdata.highThreshold = 200;

	userdata.accValue = 1024;
	userdata.minScalar0 = 160;
	userdata.minScalar1 = 30;
	userdata.minScalar2 = 50;
	userdata.maxScalar0 = 30;
	userdata.maxScalar1 = 255;
	userdata.maxScalar2 = 255;

	userdata.size = size;

	userdata.input[0] = cvCreateImage(size, IPL_DEPTH_8U, 3);
	userdata.input[1] = cvCreateImage(size, IPL_DEPTH_8U, 3);
	userdata.output[0] = cvCreateImage(size, IPL_DEPTH_8U, 3);
	userdata.output[1] = cvCreateImage(size, IPL_DEPTH_8U, 3);

	return userdata;
}

void freeSessionUserdata(Userdata *userdata) {
	cvReleaseImage(&userdata->input[0]);
	cvReleaseImage(&userdata->input[1]);
	cvReleaseImage(&userdata->output[0]);
	cvReleaseImage(&userdata->output[1]);
}

//#pragma mark - Callbacks

void mouseCallback(int event, int x, int y, int flags, void *_userdata) {
	//printf("Mouse event (%d) with flag (%d) at (%d, %d)", event, flags, x, y);
	Userdata *userdata = (Userdata *)_userdata;
	if (!userdata) {
		return;
	}

	static int down = 0;
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

	if (down) {
		userdata->click = makeClick(cvPoint(x, y), event, flags);
	}
	else {
		userdata->click = makeClick(cvPoint(0, 0), -1, 0);
	}
}

//#pragma mark - helper stuff

static IplImage *BLACK1D = NULL;
static IplImage *GRAY1D = NULL;
static IplImage *WHITE1D = NULL;

void setup(CvSize size) {
	BLACK1D = cvCreateImage(size, IPL_DEPTH_8U, 1);
	cvSet(BLACK1D, cvScalarAll(0), NULL);

	GRAY1D = cvCreateImage(size, IPL_DEPTH_8U, 1);
	cvSet(GRAY1D, cvScalarAll(127), NULL);

	WHITE1D = cvCreateImage(size, IPL_DEPTH_8U, 1);
	cvSet(WHITE1D, cvScalarAll(255), NULL);
}

//#pragma mark - filter functions

void cvClose(CvArr *src, CvArr *dst, CvArr *mask, size_t n) {
	cvCopy(src, dst, mask);
	for (size_t i = 0; i < n; i++) {
		cvErode(dst, dst, NULL, 1);
		cvDilate(dst, dst, NULL, 1);
	}
}

//#pragma mark - Operate image

void afterProcess(Userdata *userdata) {
	static int started = 0;
	if (processClick(userdata->click) == -1) { // Released button
		started = 0;
	} else { // Valid button
		//printClick(userdata->click); printf("\n");
		//printDrag(userdata->drag); printf("\n");
		if (started == 0) {
			started = 1;
			userdata->drag.start = userdata->click.origin;
		}
		userdata->drag.end = userdata->click.origin;
		userdata->circle = makeCircleFromDrag(userdata->drag, CV_RGB(255, 255, 255), 2);
	}
	//printCircle(userdata->circle); printf("\n");
	//drawCircle(userdata->input[0], userdata->circle);
	drawSquare(userdata->input[0], userdata->square);
	//printf("\n");

	int width = userdata->size.width;
	int height = userdata->size.height;
	//userdata->square = makeSquare(cvPoint(width * 3 / 4 - 40, height/2 - 40), cvSize(80, 80), CV_RGB(255, 255, 255), 2);
	drawSquare(userdata->input[0], makeSquare(cvPoint(width * 3 / 4 - 40, height-80), cvSize(80, 80), CV_RGB(255, 255, 255), 2));
}

#define COND_PRINTF(...) { if (0) printf(__VA_ARGS__); }
#define LS printf("%s::%d\n", __PRETTY_FUNCTION__, __LINE__);

void goodCorners(IplImage *src, IplImage *dst, size_t n) {
	IplImage *tmp1d = NULL;
	if (src->nChannels == 1) {
		tmp1d = cvCloneImage(src);
	} else {
		tmp1d = cvCreateImage(cvGetSize(src), src->depth, 1);
		cvCvtColor(src, tmp1d, CV_BGR2GRAY);
	}
	int corner_count = 20;
	CvPoint2D32f* corners = (CvPoint2D32f *)calloc(corner_count, sizeof(CvPoint2D32f));
	double quality_level = 0.1;
	double min_distance = 10;
	CvArr* mask = NULL;
	int block_size = 3;
	int use_harris = 0;
	double k = 0.04;
	cvGoodFeaturesToTrack(tmp1d, NULL, NULL, corners, &corner_count, quality_level, min_distance, mask, block_size, use_harris, k);

	for (int i = 0; i < corner_count; i++) {
		drawCircle(dst, makeCircle(cvPointFrom32f(corners[i]), 5, cvScalar(0, 255, 255, 0), 1));
	}
	free(corners);
}

char operateImage(Userdata *userdata) {
	if (!userdata) {
		return 0;
	}

	IplImage *image1 = userdata->input[0];
	IplImage *image2 = userdata->input[1];
	IplImage *imageOut = userdata->output[0];
	IplImage *imageOut2 = userdata->output[1];

	IplImage *tmp3d = cvCreateImage(cvGetSize(image1), image1->depth, 3);
	cvCopy(image1, tmp3d, NULL);

	CvScalar minScalar = cvScalar(userdata->minScalar0, userdata->minScalar1, userdata->minScalar2);
	CvScalar maxScalar = cvScalar(userdata->maxScalar0, userdata->maxScalar1, userdata->maxScalar2);
	//filterByHSV(tmp3d, minScalar, maxScalar, tmp3d);

	IplImage *tmp1d = cvCreateImage(cvGetSize(image1), image1->depth, 1);
	maskByHSV(tmp3d, minScalar, maxScalar, tmp1d);
	filterByVolume(tmp1d, tmp1d, tmp1d->width * tmp1d->height / 100);
	cvSmooth(tmp1d, tmp1d, CV_GAUSSIAN, 11, 0, 0, 0);
	cvThreshold(tmp1d, tmp1d, 127, 255, CV_THRESH_BINARY);
	//filterByVolume(tmp1d, tmp1d, tmp1d->width * tmp1d->height / 100);
	CVSHOW("working mask", 400, 0, 320, 240, tmp1d);

	{
		static int accValue = 512;
		if (0) {
			cvNamedWindow("memory", 0);
			cvResizeWindow("memory", 640, 240);
			cvCreateTrackbar("accValue", "memory", &accValue, 1024, NULL);
		}
		static IplImage *backgroundAcc = cvCreateImage(cvGetSize(image1), IPL_DEPTH_32F, 1);
		cvRunningAvg(tmp1d, backgroundAcc, (double)accValue / 1024, NULL);
		cvConvertScale(backgroundAcc, tmp1d, 1, 0);
		//cvThreshold(tmp1d, tmp1d, 224, 255, CV_THRESH_BINARY);
		CVSHOW("memory mask", 800, 0, 320, 240, tmp1d);
	}

	cvCopy2(tmp3d, tmp3d, tmp1d);
	//goodCorners(tmp1d, tmp3d, 30);
	//houghLines(tmp3d, tmp3d);
	//doFaces(image1, tmp3d, 4);


	IplImage *cannyMask = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 1);
	cvCvtColor(tmp3d, cannyMask, CV_BGR2GRAY);
	cvSmooth(cannyMask, cannyMask, CV_GAUSSIAN, 5, 0, 0, 0);
	unsigned int lThreshold = 50;
	unsigned int hThreshold = 200;
	cvCanny(cannyMask, cannyMask, lThreshold, hThreshold, 3);
	CVSHOW("canny mask", 400, 300, 320, 240, cannyMask);
	cvReleaseImage(&cannyMask);

#define USE_FACE 0
#if USE_FACE
	static CvRect face = cvRect(0, 0, 10, 10);
	static int tick = 0;
	//userdata->timestep = 100;
	if ((tick %= 10) == 0) {
		double scale = 4;
		CvRect *faces = NULL;
		double t = 0;
		int facesCount = findFaces(tmp3d, &faces, scale, &t);
		face = (facesCount != 0) ? faces[0] : face;
		printf("%d face(s) detected in %g ms :: 1st face at {%d,%d,%d,%d}", facesCount, t, face.x, face.y, face.width, face.height);
		//drawFaces(tmp3d, 1, &face);
		drawFaces(tmp3d, facesCount, faces);
		free(faces);
	}
	tick++;

#endif
#if USE_FACE && USE_SUBIMAGE
	//face extraction
	IplImage *subimage = createSubArray(tmp3d, face);
	cvNamedWindow(CONTROL_WINDOW  "face", 0);
	cvResizeWindow(CONTROL_WINDOW "face", subimage->width, subimage->height);
	cvShowImage(CONTROL_WINDOW    "face", subimage);
#endif
#if USE_SUBIMAGE && USE_HISTOGRAM
	//face histogram
	IplImage *subimage2 = cvCloneImage(subimage);
	cvCvtColor(subimage2, subimage2, CV_BGR2HSV);
	size_t binsCount = 0;
	size_t *bins = NULL;
	//printf("%d (%p)", binsCount, &bins);
	calcularHistograma(subimage2, &binsCount, &bins);
	//printf(" ret=%d %d (%p)", ret, binsCount, bins);
	CvScalar maxValues;
	if (bins) {
		for (size_t i = 0; i < binsCount; i++) {
			size_t idx = 0;
			for (size_t j = 0; j < 256; j++) {
				if (bins[i * 256 + j] > bins[idx]) {
					idx = j;
				}
			}
			maxValues.val[i] = idx;
		}
		free(bins);
	}

	if (subimage->width > 10 && subimage->height > 10)
	graficarHistograma(subimage, binsCount, bins);
	cvNamedWindow(CONTROL_WINDOW  "42", 0);
	cvResizeWindow(CONTROL_WINDOW "42", subimage->width, subimage->height);
	cvShowImage(CONTROL_WINDOW    "42", subimage);
#endif
#if USE_SUBIMAGE && 0
	int minH = (int)maxValues.val[0] - 20;
	int maxH = (int)maxValues.val[0] + 20;
	int minS = (int)maxValues.val[1] - 20;
	int maxS = (int)maxValues.val[1] + 20;
	int minV = (int)maxValues.val[2] - 20;
	int maxV = (int)maxValues.val[2] + 20;

	minH = minH < 0 ? 180 - minH : minH;
	maxH = maxH > 180 ? maxH - 180 : maxH;

	printf("%d,%d,%d %d,%d,%d", minH, minS, minV, maxH, maxS, maxV);

	filterByHSV(subimage2, cvScalar(minH, minS, minV, 0), cvScalar(maxH, maxS, maxV, 0), subimage2);
	filterByHSV(subimage2, minScalar, maxScalar, subimage2);
	cvCvtColor(subimage2, subimage2, CV_HSV2BGR);

	cvNamedWindow(CONTROL_WINDOW "41", 0);
	cvResizeWindow(CONTROL_WINDOW "41", subimage2->width, subimage2->height);
	cvShowImage(CONTROL_WINDOW "41", subimage2);
#endif
#if USE_SUBIMAGE
	cvReleaseImage(&subimage);
	cvReleaseImage(&subimage2);
#endif

	cvCopy(image1, image2, NULL);
	cvCopy(imageOut, imageOut2, NULL);
	cvCopy(tmp3d, imageOut, NULL);

	cvReleaseImage(&tmp3d);

	//afterProcess(userdata);

	printf("\r");
	return 0;
}
