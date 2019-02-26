#ifndef DRAWING_H
#define DRAWING_H

#include "common.h"

DIP_EXTERN_BEGIN

CvScalar cvScalarRGBFromHSV(CvScalar hsv);

void drawBadge(CvArr *img, char *string, CvScalar fontColor, double fontSize, CvPoint badgeCenter, CvScalar badgeColor);

DIP_EXTERN_END

#include "click.h"

DIP_EXTERN_BEGIN

char PointInRect(CvPoint point, CvSize size);

typedef struct Circle {
	CvPoint center;
	int radius;
	CvScalar color;
	int thickness;
} Circle;

struct Circle makeCircle(CvPoint center, int radius, CvScalar color, int thickness);
struct Circle makeCircleFromDrag(Drag drag, CvScalar color, int thickness);
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

DIP_EXTERN_END

#endif /* DRAWING_H */
