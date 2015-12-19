#include "geometry.h"
#include <stdio.h>

char pointIsInRect(CvPoint point, CvSize size) {
	return !(point.y < 0 || point.y >= size.height || point.x < 0 || point.x >= size.width);
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
