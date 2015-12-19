#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <opencv2/core/core_c.h>
#include "click.h"

char PointInRect(CvPoint point, CvSize size);



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

#endif /* GEOMETRY_H */
