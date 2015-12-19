//#include "stdafx.h"
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

//#pragma mark - controls

#define SHOW_CONTROLS 1
#define CONTROLS_WIDTH 400
#define CONTROLS_HEIGHT 120

void setupWindows(Userdata *userdata) {
#if SHOW_CONTROLS
	//cvCreateTrackbar("kernel size", CONTROL_WINDOW, &(userdata.kernelSize), 15, trackbarCallback);

	cvNamedWindow(CONTROL_WINDOW "1", 0);
	cvResizeWindow(CONTROL_WINDOW "1", CONTROLS_WIDTH, CONTROLS_HEIGHT);

	cvCreateTrackbar("min 0", CONTROL_WINDOW "1", &(userdata->minScalar0), 180, trackbarCallback);
	cvCreateTrackbar("min 1", CONTROL_WINDOW "1", &(userdata->minScalar1), 255, trackbarCallback);
	cvCreateTrackbar("min 2", CONTROL_WINDOW "1", &(userdata->minScalar2), 255, trackbarCallback);

	cvNamedWindow(CONTROL_WINDOW "2", 0);
	cvResizeWindow(CONTROL_WINDOW "2", CONTROLS_WIDTH, CONTROLS_HEIGHT);

	cvCreateTrackbar("max 0", CONTROL_WINDOW "2", &(userdata->maxScalar0), 180, trackbarCallback);
	cvCreateTrackbar("max 1", CONTROL_WINDOW "2", &(userdata->maxScalar1), 255, trackbarCallback);
	cvCreateTrackbar("max 2", CONTROL_WINDOW "2", &(userdata->maxScalar2), 255, trackbarCallback);

	//cvCreateTrackbar("value", CONTROL_WINDOW, &(userdata.value), (int)sqrt(cam_height * cam_width), trackbarCallback);
#endif
}
void destroyWindows(Userdata *userdata) {
#if SHOW_CONTROLS
	cvDestroyWindow(CONTROL_WINDOW "1");
	cvDestroyWindow(CONTROL_WINDOW "2");
#endif
}

//#pragma mark - Callbacks

void trackbarCallback(int val) {}

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

char operateImage(Userdata *userdata) {
	if (!userdata) {
		return 0;
	}

	IplImage *image1 = userdata->input[0];
	IplImage *image2 = userdata->input[1];
	IplImage *imageOut = userdata->output[0];
	IplImage *imageOut2 = userdata->output[1];

	IplImage *tmp3d = cvCreateImage(cvGetSize(image1), IPL_DEPTH_8U, 3);
	cvCopy(image1, tmp3d, NULL);

	CvScalar minScalar = cvScalar(userdata->minScalar0, userdata->minScalar1, userdata->minScalar2);
	CvScalar maxScalar = cvScalar(userdata->maxScalar0, userdata->maxScalar1, userdata->maxScalar2);
	filterByHSV(tmp3d, minScalar, maxScalar, tmp3d);

#if 0
	static CvRect face = cvRect(0, 0, 10, 10);
	static int tick = 0;
	if ((tick %= 10) == 0) {
		//userdata->timestep = 100;
		double scale = 4;
		CvRect *faces = NULL;
		double t = 0;
		int facesCount = findFaces(tmp3d, &faces, scale, &t);
		face = (facesCount != 0) ? faces[0] : face;
		free(faces);
		printf("%d face(s) detected in %g ms :: 1st face at {%d,%d,%d,%d}", facesCount, t, face.x, face.y, face.width, face.height);
		drawFaces(tmp3d, 1, &face);
	}
	tick++;

#endif
#if 0
	//face extraction
	IplImage *subimage = createSubArray(tmp3d, face);
	cvNamedWindow(CONTROL_WINDOW  "face", 0);
	cvResizeWindow(CONTROL_WINDOW "face", subimage->width, subimage->height);
	cvShowImage(CONTROL_WINDOW    "face", subimage);
#endif
#if 0
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
#endif
#if 0
	if (subimage->width > 10 && subimage->height > 10)
	graficarHistograma(subimage, binsCount, bins);
	cvNamedWindow(CONTROL_WINDOW  "42", 0);
	cvResizeWindow(CONTROL_WINDOW "42", subimage->width, subimage->height);
	cvShowImage(CONTROL_WINDOW    "42", subimage);
#endif

#if 0
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

#if 0
	cvReleaseImage(&subimage);
	cvReleaseImage(&subimage2);
#endif

	cvCopy(image1, image2, NULL);
	cvCopy(imageOut, imageOut2, NULL);
	cvCopy(tmp3d, imageOut, NULL);

	//cvReleaseImage(&tmp1d);
	cvReleaseImage(&tmp3d);

	//afterProcess(userdata);

	printf("\r");
	return 0;
}