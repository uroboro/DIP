#include "stdafx.h"
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
void destroyWindows() {
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

char pointIsInRect(CvPoint point, CvSize size) {
	return !(point.y < 0 || point.y >= size.height || point.x < 0 || point.x >= size.width);
}

//#pragma mark - click functions

Click makeClick(CvPoint origin, int event, int flags) {
	Click click;
	memset(&click, 0, sizeof(Click));
	click.origin = origin;
	click.event = event;
	click.flags = flags;

	click.down = -1;

	return click;
}

int printClick(Click click) {
	return printf("<Click origin={%d, %d} event=%d flags=%d down=%d(%d) edges=%d%d>", click.origin.x, click.origin.y, click.event, click.flags, click.down, click.down_p, click.down_edge, click.up_edge);
}

int processClick(Click click) {
	switch (click.event) {
	case -1:
		return -1;
	case CV_EVENT_LBUTTONDOWN:
	case CV_EVENT_RBUTTONDOWN:
	case CV_EVENT_MBUTTONDOWN:
		click.down = 1;
		break;
	case CV_EVENT_LBUTTONUP:
	case CV_EVENT_RBUTTONUP:
	case CV_EVENT_MBUTTONUP:
		click.down = 0;
		break;
	default:
		break;
	}

	if (click.down != click.down_p && click.down == 0) {
		click.down_edge = 1;
	}
	else {
		click.down_edge = 0;
	}
	if (click.down != click.down_p && click.down == 1) {
		click.up_edge = 1;
	}
	else {
		click.up_edge = 0;
	}

	click.down_p = click.down;

	return 0;
}

//#pragma mark - drag functions

int printDrag(Drag drag) {
	return printf("<Drag start={%d, %d} end={%d, %d}>", drag.start.x, drag.start.y, drag.end.x, drag.end.y);
}

//#pragma mark - circle functions

Circle makeCircle(CvPoint center, int radius, CvScalar color, int thickness) {
	Circle circle;
	memset(&circle, 0, sizeof(Circle));
	circle.center = center;
	circle.radius = radius;
	circle.color = color;
	circle.thickness = thickness;

	return circle;
}

Circle makeCircleFromDrag(Drag drag, CvScalar color, int thickness) {
	int X = drag.end.x - drag.start.x;
	int Y = drag.end.y - drag.start.y;
	int radius = (int)sqrt(X * X + Y * Y);
	return makeCircle(cvPoint(drag.start.x, drag.start.y), radius, color, thickness);
}

int printCircle(Circle circle) {
	return printf("<Circle center={%d, %d} radius=%d color={%d, %d, %d} thickness=%d>", circle.center.x, circle.center.y, circle.radius, (int)circle.color.val[0], (int)circle.color.val[1], (int)circle.color.val[2], circle.thickness);
}

void drawCircle(CvArr *img, Circle circle) {
	cvCircle(img, circle.center, circle.radius, circle.color, circle.thickness, 8, 0);
}

//pragma mark - square functions

Square makeSquare(CvPoint origin, CvSize size, CvScalar color, int thickness) {
	Square square;
	square.origin = origin;
	square.size = size;
	square.color = color;
	square.thickness = thickness;

	return square;
}

Square makeSquareFromDrag(Drag drag, CvScalar color, int thickness) {
	int X = drag.end.x - drag.start.x;
	int Y = drag.end.y - drag.start.y;
	return makeSquare(drag.start, cvSize(X, Y), color, thickness);
}

int printSquare(Square square) {
	return printf("<Square origin={%d, %d} size={%d, %d} color={%d, %d, %d} thickness=%d>", square.origin.x, square.origin.y, square.size.width, square.size.height, (int)square.color.val[0], (int)square.color.val[1], (int)square.color.val[2], square.thickness);
}

void drawSquare(CvArr *img, Square square) {
	CvPoint pt2 = cvPoint(square.origin.x + square.size.width, square.origin.y + square.size.height);
	cvDrawRect(img, square.origin, pt2, square.color, square.thickness, 8, 0);
}

//#pragma mark - filter functions

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
				//if (pointIsInRect(x, y, width, height))
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

void cvClose(CvArr *src, CvArr *dst, CvArr *mask, size_t n) {
	cvCopy(src, dst, mask);
	for (size_t i = 0; i < n; i++) {
		cvErode(dst, dst, NULL, 1);
		cvDilate(dst, dst, NULL, 1);
	}
}

void cvCopy2(CvArr *src, CvArr *dst, CvArr *mask) {
	IplImage *tmp1dB = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dG = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dR = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);

	cvSplit(src, tmp1dB, tmp1dG, tmp1dR, NULL);

	cvAnd(tmp1dB, mask, tmp1dB);
	cvAnd(tmp1dG, mask, tmp1dG);
	cvAnd(tmp1dR, mask, tmp1dR);

	cvMerge(tmp1dB, tmp1dG, tmp1dR, NULL, dst);

	cvReleaseImage(&tmp1dB);
	cvReleaseImage(&tmp1dG);
	cvReleaseImage(&tmp1dR);
}

int filterByHSV(IplImage *src, CvScalar minHSV, CvScalar maxHSV, IplImage *dst) {
	IplImage *tmp3d = cvCloneImage(src);
	cvSmooth(tmp3d, tmp3d, CV_GAUSSIAN, 13, 0, 0, 0);

	cvCvtColor(tmp3d, tmp3d, CV_BGR2HSV);
	IplImage *tmp1dH_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dS_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage *tmp1dV_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvSplit(tmp3d, tmp1dH_mask, tmp1dS_mask, tmp1dV_mask, NULL);

	//printf("\rmin: %03d,%03d,%03d", (int)minHSV.val[0], (int)minHSV.val[1], (int)minHSV.val[2]);
	//printf("\tmax: %03d,%03d,%03d", (int)maxHSV.val[0], (int)maxHSV.val[1], (int)maxHSV.val[2]);

	if (minHSV.val[0] < maxHSV.val[0]) {
		cvInRangeS(tmp1dH_mask, cvScalar(minHSV.val[0], 0, 0), cvScalar(maxHSV.val[0], 0, 0), tmp1dH_mask);
	} else {
		IplImage *tmp1d = cvCloneImage(tmp1dH_mask);
		cvInRangeS(tmp1dH_mask, cvScalar(0, 0, 0), cvScalar(maxHSV.val[0], 0, 0), tmp1d);
		cvInRangeS(tmp1dH_mask, cvScalar(minHSV.val[0], 0, 0), cvScalar(255, 0, 0), tmp1dH_mask);
		cvOr(tmp1d, tmp1dH_mask, tmp1dH_mask, NULL);
		cvReleaseImage(&tmp1d);
	}

	cvInRangeS(tmp1dS_mask, cvScalar(minHSV.val[1], 0, 0), cvScalar(maxHSV.val[1], 0, 0), tmp1dS_mask);
	cvInRangeS(tmp1dV_mask, cvScalar(minHSV.val[2], 0, 0), cvScalar(maxHSV.val[2], 0, 0), tmp1dV_mask);

	IplImage *tmp1d_mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvSet(tmp1d_mask, cvScalarAll(255), NULL);
	cvAnd(tmp1d_mask, tmp1dH_mask, tmp1d_mask, NULL);
	cvAnd(tmp1d_mask, tmp1dS_mask, tmp1d_mask, NULL);
	cvAnd(tmp1d_mask, tmp1dV_mask, tmp1d_mask, NULL);

	cvReleaseImage(&tmp1dH_mask);
	cvReleaseImage(&tmp1dS_mask);
	cvReleaseImage(&tmp1dV_mask);

	cvClose(tmp1d_mask, tmp1d_mask, NULL, 2);

#define CONTROLS_WIDTHA  640/2
#define CONTROLS_HEIGHTA 480/2
#if 1
	cvNamedWindow(CONTROL_WINDOW  "4", 0);
	cvResizeWindow(CONTROL_WINDOW "4", CONTROLS_WIDTHA, CONTROLS_HEIGHTA);
	cvShowImage(CONTROL_WINDOW    "4", tmp1d_mask);
#endif

	cvCopy2(src, dst, tmp1d_mask);

	cvReleaseImage(&tmp1d_mask);

	return 0;
}

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

	//Actúo en función de la cantidad de colores de la imágen
	if (channels == 1) {
		float range[] = { 0, 256 };
		float* ranges[] = { range };

		float max = 0.0;
		float w_scale = 0.0;

		CvHistogram *hist_bw = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
		cvCalcHist(&src, hist_bw, 0, NULL);
		//float max_value = 0.0;
		//cvGetMinMaxHistValue(hist_bw, 0, &max_value, 0, 0);
		//cvScale(hist_bw->bins, hist_bw->bins, (float)(src->width*src->height) / max_value, 0);

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

	//Actúo en función de la cantidad de colores de la imágen
	if (dst->nChannels == 1) {
		size_t max_value = 0;
		for (size_t i = 0; i < binsCount * hist_size; i++) {
			max_value = (bins[i] > max_value) ? bins[i] : max_value;
		}
		for (size_t i = 0; i < binsCount * hist_size; i++) {
			bins[i] /= max_value;
		}
		float w_scale = ((float)dst->width) / hist_size;

		//Graficar en la imagen
		for (int i = 0; i < hist_size; i++) {
			cvLine(dst,
				cvPoint(binsCount * hist_size + (int)(i       * w_scale), dst->height - bins[i]),
				cvPoint(binsCount * hist_size + (int)((i + 1) * w_scale), dst->height - bins[i]),
				hist_color, 2, 8, 0);
		}

		//printf("Scale bw: %4.2f pixels per 100 units\r", max_value * 100 / ((float)ImagenHistorial->height));
	} else if (dst->nChannels == 3) {
		IplImage *channelA = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);
		IplImage *channelB = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);
		IplImage *channelC = cvCreateImage(cvGetSize(dst), IPL_DEPTH_8U, 1);
		cvSplit(dst, channelA, channelB, channelC, NULL);

		size_t mybins[256];
		size_t max_value = 0;
		for (size_t i = 0; i < hist_size; i++) {
			max_value = (bins[i] > max_value) ? bins[i] : max_value;
		}
		for (size_t i = 0; i < binsCount * hist_size; i++) {
			bins[i] /= max_value;
		}
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

int findFaces(CvArr* src, CvRect **rects, double scale, double *t)
{
	CvSize srcSize = cvGetSize(src);
	IplImage *gray = cvCreateImage(srcSize, IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_BGR2GRAY);

	double fx = 1 / scale;
	srcSize.width = (int)cvRound(fx * srcSize.width);
	srcSize.height = (int)cvRound(fx * srcSize.height);

	IplImage *smallImg = cvCreateImage(srcSize, IPL_DEPTH_8U, 1);
	cvResize(gray, smallImg, CV_INTER_LINEAR);
	cvReleaseImage(&gray);

	cvEqualizeHist(smallImg, smallImg);

	char frontalFaceFile[] = RESOURCES "haarcascade_frontalface_alt.xml";

	cv::CascadeClassifier cascade;
	if (!cascade.load(frontalFaceFile)) {
		printf("ERROR: Could not load classifier cascade\r");
		return -1;
	}
	std::vector<cv::Rect> faces, faces2;
	*t = (double)cvGetTickCount();
	cascade.detectMultiScale(smallImg, faces,
		1.1, 2, 0
		//|CASCADE_FIND_BIGGEST_OBJECT
		//|CASCADE_DO_ROUGH_SEARCH
		| cv::CASCADE_SCALE_IMAGE,
		cv::Size(30, 30));

	*t = (double)cvGetTickCount() - *t;
	*t /= ((double)cvGetTickFrequency() * 1000);

	*rects = (CvRect *)calloc(faces.size() + 1, sizeof(CvRect));
	for (size_t i = 0; i < faces.size(); i++) {
		CvRect face = faces[i];
		face = cvRect(cvRound(face.x * scale), cvRound(face.y * scale), cvRound(face.width * scale), cvRound(face.height * scale));
		(*rects)[i] = face;
	}

	return faces.size();
}

void drawFaces(CvArr *dst, size_t facesCount, CvRect *faces) {
	const static CvScalar colors[] =
	{
		cvScalar(255, 0, 0),
		cvScalar(255, 128, 0),
		cvScalar(255, 255, 0),
		cvScalar(0, 255, 0),
		cvScalar(0, 128, 255),
		cvScalar(0, 255, 255),
		cvScalar(0, 0, 255),
		cvScalar(255, 0, 255)
	};

	for (size_t i = 0; i < facesCount; i++) {
		CvRect r = faces[i];
		CvScalar color = colors[i % 8];

		cvRectangle(dst, cvPoint(r.x, r.y),
			cvPoint(r.x + r.width - 1, r.y + r.height - 1),
			color, 3, 8, 0);
	}
}


IplImage* createSubArray(IplImage *src, CvRect rect) {
	CvRect prevRect = cvGetImageROI(src);
	cvSetImageROI(src, rect);
	IplImage *dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	cvCopy(src, dst, NULL);
	cvSetImageROI(src, prevRect);
	return dst;
}

#if 1
char operateImage(Userdata *userdata) {
	if (!userdata) {
		return 0;
	}

	IplImage *image1 = userdata->input[0];
	IplImage *image2 = userdata->input[1];
	IplImage *imageOut = userdata->output[0];
	IplImage *imageOut2 = userdata->output[1];

	static int color_mode = 4;
	static int smooth_mode = 0;
	static int otsu_mode = 0;
	static int close_mode = 0;
	static int canny_mode = 0;
	static int contour_mode = 0;
	static int hsv_mode = 0;
	static int save_img = 0;
	static int history_mode = 0;

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
	case 'h':
		hsv_mode = !hsv_mode;
		break;
	case 'H':
		history_mode = !history_mode;
		break;
	case 'S':
		save_img = 1;
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
	cvCopy(image1, tmp3d, NULL);

	filterByHSV(tmp3d, minScalar, maxScalar, tmp3d);
	
	static int tick = 0;
	static CvRect face = cvRect(0,0,1,1);
	if ((tick %= 10) == 0) {
		//userdata->timestep = 100;
		double scale = 4;
		CvRect *faces = NULL;
		double t = 0;
		int facesCount = findFaces(tmp3d, &faces, scale, &t);
		face = (facesCount != 0) ? faces[0] : face;
		free(faces);
		//printf("%d face(s) detected in %g ms :: 1st face at {%d,%d,%d,%d}", facesCount, t, face.x, face.y, face.width, face.height);
		drawFaces(tmp3d, 1, &face);
	}
	tick++;

	//face extraction
	IplImage *subimage = createSubArray(tmp3d, face);
	cvNamedWindow(CONTROL_WINDOW  "face", 0);
	cvResizeWindow(CONTROL_WINDOW "face", subimage->width, subimage->height);
	cvShowImage(CONTROL_WINDOW    "face", subimage);

	//face histogram
	IplImage *subimage2 = cvCloneImage(subimage);
	cvCvtColor(subimage2, subimage2, CV_BGR2HSV);
	size_t binsCount = 0;
	size_t *bins = NULL;
	//printf("%d (%p)", binsCount, &bins);
	size_t ret = calcularHistograma(subimage2, &binsCount, &bins);
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

	cvReleaseImage(&subimage);
	cvReleaseImage(&subimage2);
	
	cvCopy(image1, image2, NULL);
	cvCopy(imageOut, imageOut2, NULL);
	cvCopy(tmp3d, imageOut, NULL);

	//cvReleaseImage(&tmp1d);
	//cvReleaseImage(&tmp3d);

	//afterProcess(userdata);

	printf("\r");
	return 0;
}

#else
char operateImage(Userdata *userdata) {
	if (!userdata) {
		return 0;
	}

	IplImage *image1 = userdata->input[0];
	IplImage *image2 = userdata->input[1];
	IplImage *imageOut = userdata->output[0];
	IplImage *imageOut2 = userdata->output[1];

	static int color_mode = 4;
	static int smooth_mode = 0;
	static int otsu_mode = 0;
	static int close_mode = 0;
	static int canny_mode = 0;
	static int contour_mode = 0;
	static int hsv_mode = 0;
	static int save_img = 0;
	static int history_mode = 0;

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
	case 'h':
		hsv_mode = !hsv_mode;
		break;
	case 'H':
		history_mode = !history_mode;
		break;
	case 'S':
		save_img = 1;
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

	COND_PRINTF("                                                                                                 \r");

	char img_full_channel = 0;
	switch (color_mode) {
	case 0:
		COND_PRINTF("Gray");
		cvCvtColor(image1, tmp1d, CV_BGR2GRAY);
		break;
	case 1: // Hue mode
		COND_PRINTF("Hue");
		cvCvtColor(image1, tmp3d, CV_BGR2HSV);
		cvSplit(tmp3d, tmp1d, NULL, NULL, NULL);
		break;
	case 2: // Saturation mode
		COND_PRINTF("Saturation");
		cvCvtColor(image1, tmp3d, CV_BGR2HSV);
		cvSplit(tmp3d, NULL, tmp1d, NULL, NULL);
		break;
	case 3: // Brightness mode
		COND_PRINTF("Brightness");
		cvCvtColor(image1, tmp3d, CV_BGR2HSV);
		cvSplit(tmp3d, NULL, NULL, tmp1d, NULL);
		break;
	case 4: // 
		COND_PRINTF("Color");
		img_full_channel = 1;
		break;
	}

	//filterByVolume(tmp1d, tmp1d, value);
	if (img_full_channel) { // Image has 3 channel
#if 0
		cvRunningAvg(image1, backgroundAcc, (double)userdata->accValue / 1024, NULL);
		cvConvertScale(backgroundAcc, background, 1, 0);
		cvNamedWindow(CONTROL_WINDOW "41", 0);
		cvResizeWindow(CONTROL_WINDOW "41", 640 / 2, 480 / 2);
		cvShowImage(CONTROL_WINDOW "41", background);
		cvCreateTrackbar("accValue", CONTROL_WINDOW "41", &(userdata->accValue), 1024, trackbarCallback);

#endif
		filterByHSV(image1, minScalar, maxScalar, tmp3d);
		if (history_mode) {
			cvCopy(image1, tmp3d, NULL);
			cvCopy(image1, tmp3d2, NULL);
			//cvCvtColor(image1, tmp3d, CV_BGR2HSV);

			//CvRect rect = cvRect(userdata->size.width * 3 / 4 - 40, userdata->size.height / 2 - 40, 80, 80);
			//CvRect rect = cvRect(userdata->size.width * 1 / 4 - 40, userdata->size.height / 2 - 40, userdata->size.width * 3 / 4, 80);
			CvRect rect = cvRect(userdata->square.origin.x, userdata->square.origin.y, userdata->square.size.width, userdata->square.size.height);
			cvSetImageROI(tmp3d, rect);
			GraficarHistograma(tmp3d, tmp3d2);
			cvResetImageROI(tmp3d);

			cvCopy(tmp3d2, tmp3d, NULL);
		}
		else {
			cvCopy(image1, tmp3d, NULL);
		}
	}
	else { // Image has 1 channel

		cvSmooth(tmp1d, tmp1d, CV_GAUSSIAN, 5, 0, 0, 0);

		if (otsu_mode) { // Apply Otsu's method
			COND_PRINTF(", Otsu");
			cvThreshold(tmp1d, tmp1d, 0, 255, CV_THRESH_OTSU);
		}

		if (smooth_mode) { // Apply Gaussian smoothing
			COND_PRINTF(", Gauss");
			cvSmooth(tmp1d, tmp1d, CV_GAUSSIAN, 5, 0, 0, 0);
		}

		if (close_mode) {
			COND_PRINTF(", closE");
			int n = kernelSize;
			cvErode(tmp1d, tmp1d, NULL, n);
			cvDilate(tmp1d, tmp1d, NULL, n);
		}

		if (canny_mode) { // Apply Canny's method
			COND_PRINTF(", Canny");
			cvCanny(tmp1d, tmp1d, lowThreshold, highThreshold, 3);
			cvDilate(tmp1d, tmp1d, NULL, 1);
			cvErode(tmp1d, tmp1d, NULL, 1);
		}

		cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);

		if (contour_mode) {
			COND_PRINTF(", contours(b)");
			CvMemStorage *storage = cvCreateMemStorage(0);
			CvSeq *contours = NULL;
			int n = cvFindContours(tmp1d, storage, &contours, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
			//COND_PRINTF(", (" << n <<","<< contours->total <<")contours");
			for (int i = 0; contours != NULL; contours = contours->h_next, i++) {
				int cc = (int)((float)(255 * i) / contours->total);
				CvScalar colorpx = CV_RGB((cc) % 256, (cc + 256 / 3) % 256, (cc + 256 * 2 / 3) % 256);
				cvDrawContours(tmp3d, contours, colorpx, CV_RGB(0, 0, 0), -1, CV_FILLED, 8, cvPoint(0, 0));
			}
		}

	}

	COND_PRINTF("\r");

	cvCopy(image1, image2, NULL);
	cvCopy(imageOut, imageOut2, NULL);
	cvCopy(tmp3d, imageOut, NULL);

	//cvReleaseImage(&tmp1d);
	//cvReleaseImage(&tmp3d);
	//cvReleaseImage(&tmp3d2);

	afterProcess(userdata);

	if (save_img) {
		save_img = 0;
		cvSaveImage(RESOURCES "output.png", imageOut);
	}

	return 0;
}
#endif