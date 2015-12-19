#include <stdio.h>
#include <opencv2/core/core_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/objdetect/objdetect.hpp>

#define TRY(...) { try { __VA_ARGS__ } catch (cv::Exception *e) { fprintf(stderr, "%s", e->what()); } }

#define RESOURCES "C:\\Users\\uroboro\\Documents\\GitHub\\DIP\\DIP\\resources\\"

#define INPUT_WINDOW	"Input Window"
#define OUTPUT_WINDOW	"Output Window"
#define CONTROL_WINDOW	"Control Window"

char PointInRect(CvPoint point, CvSize size);

typedef struct _click {
	CvPoint origin;
	int event;
	int flags;

	int down;
	int down_p;
	int down_edge;
	int up_edge;
} Click;

Click makeClick(CvPoint origin, int event, int flags);
int printClick(Click click);

typedef struct _drag {
	CvPoint start;
	CvPoint end;
} Drag;

int printDrag(Drag drag);

typedef struct _circle {
	CvPoint center;
	int radius;
	CvScalar color;
	int thickness;
} Circle;

Circle makeCircle(CvPoint center, int radius, CvScalar color, int thickness);
Circle makeCircleFromDrag(Drag drag, CvScalar color, int thickness);
int printCircle(Circle circle);

void drawCircle(CvArr *img, Circle circle);

typedef struct _square {
	CvPoint origin;
	CvSize size;
	CvScalar color;
	int thickness;
} Square;

Square makeSquare(CvPoint origin, CvSize size, CvScalar color, int thickness);
Square makeSquareFromDrag(Drag drag, CvScalar color, int thickness);
int printSquare(Square square);

void drawSquare(CvArr *img, Square square);


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
void destroyWindows();

char operateImage(Userdata *userdata);

void mouseCallback(int event, int x, int y, int flags, void* userdata);
void trackbarCallback(int val);

int filterByVolume(IplImage *src, IplImage *dst, long minVolume);
int filterByHSV(IplImage *src, CvScalar minHSV, CvScalar maxHSV, IplImage *dst);

void GraficarHistograma(IplImage *Imagen, IplImage *ImagenHistorial);