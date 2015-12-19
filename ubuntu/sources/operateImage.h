#include <stdio.h>
#include <opencv2/core/core_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/objdetect/objdetect.hpp>

#define TRY(...) { try { __VA_ARGS__ } catch (cv::Exception *e) { fprintf(stderr, "%s", e->what()); } }

#define INPUT_WINDOW	"Input Window"
#define OUTPUT_WINDOW	"Output Window"
#define CONTROL_WINDOW	"Control Window"

#include "click.h"
#include "geometry.h"
#include "histogram.h"
#include "filter_hsv.h"
#include "face_detection.h"
#include "fixes.h"

typedef struct _userdata {
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

Userdata getSessionUserdata(CvSize size);
void freeSessionUserdata(Userdata *userdata);

void setupWindows(Userdata *userdata);
void destroyWindows(Userdata *userdata);

char operateImage(Userdata *userdata);

void mouseCallback(int event, int x, int y, int flags, void* userdata);
void trackbarCallback(int val);

int filterByVolume(IplImage *src, IplImage *dst, long minVolume);
