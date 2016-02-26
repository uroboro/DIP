#ifndef DRAWING_H
#define DRAWING_H

#include <opencv2/core/core_c.h>

#include "common.h"
#include "click.h"

DIP_EXTERN char PointInRect(CvPoint point, CvSize size);



DIP_EXTERN typedef struct _circle {
	CvPoint center;
	int radius;
	CvScalar color;
	int thickness;
} Circle;

DIP_EXTERN Circle makeCircle(CvPoint center, int radius, CvScalar color, int thickness);
DIP_EXTERN Circle makeCircleFromDrag(Drag drag, CvScalar color, int thickness);
DIP_EXTERN int printCircle(Circle circle);

DIP_EXTERN void drawCircle(CvArr *img, Circle circle);



DIP_EXTERN typedef struct _square {
	CvPoint origin;
	CvSize size;
	CvScalar color;
	int thickness;
} Square;

DIP_EXTERN Square makeSquare(CvPoint origin, CvSize size, CvScalar color, int thickness);
DIP_EXTERN Square makeSquareFromDrag(Drag drag, CvScalar color, int thickness);
DIP_EXTERN int printSquare(Square square);

DIP_EXTERN void drawSquare(CvArr *img, Square square);

#endif /* DRAWING_H */
