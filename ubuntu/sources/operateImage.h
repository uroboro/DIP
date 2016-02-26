#ifndef OPERATEIMAGE_H
#define OPERATEIMAGE_H

#include <stdio.h>
#include <opencv2/core/core_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#define TRY(...) { try { __VA_ARGS__ } catch (cv::Exception *e) { fprintf(stderr, "%s", e->what()); } }

#define INPUT_WINDOW	"Input Window"
#define OUTPUT_WINDOW	"Output Window"

#include "common.h"
#include "click.h"
#include "drawing.h"
#include "histogram.h"
#include "filter_hsv.h"
#include "face_detection.h"
#include "fixes.h"

DIP_EXTERN typedef struct _userdata {
	int timestep;
	int key;

	int value;
	int kernelSize;

	int lowThreshold;
	int highThreshold;

	CvSize size;
	Click click;
	Drag drag;
	Circle circle;
	Square square;

	int accValue;

	int minScalar0;
	int minScalar1;
	int minScalar2;
	int maxScalar0;
	int maxScalar1;
	int maxScalar2;

	IplImage *input[2];
	IplImage *output[2];
} Userdata;

DIP_EXTERN Userdata getSessionUserdata(CvSize size);
DIP_EXTERN void freeSessionUserdata(Userdata *userdata);

DIP_EXTERN void cvClose(CvArr *src, CvArr *dst, CvArr *mask, size_t n);
DIP_EXTERN char operateImage(Userdata *userdata);

DIP_EXTERN void mouseCallback(int event, int x, int y, int flags, void* userdata);
DIP_EXTERN void trackbarCallback(int val);

DIP_EXTERN int filterByVolume(IplImage *src, IplImage *dst, long minVolume);

#endif /* OPERATEIMAGE_H */
