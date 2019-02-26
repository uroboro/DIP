#include <stdio.h>
#include <math.h>

#include "drawing.h"

static void hsvtorgb(unsigned char *r, unsigned char *g, unsigned char *b, unsigned char h, unsigned char s, unsigned char v) {
	unsigned char region, fpart, p, q, t;

	if (s == 0) {
		/* color is grayscale */
		*r = *g = *b = v;
		return;
	}

	/* make hue 0-5 */
	region = h / 43;
	/* find remainder part, make it from 0-255 */
	fpart = (h - (region * 43)) * 6;

	/* calculate temp vars, doing integer multiplication */
	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * fpart) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;

	/* assign temp vars based on color cone region */
	switch(region) {
		case 0:
			*r = v; *g = t; *b = p; break;
		case 1:
			*r = q; *g = v; *b = p; break;
		case 2:
			*r = p; *g = v; *b = t; break;
		case 3:
			*r = p; *g = q; *b = v; break;
		case 4:
			*r = t; *g = p; *b = v; break;
		default:
			*r = v; *g = p; *b = q; break;
	}

	return;
}

CvScalar cvScalarRGBFromHSV(CvScalar hsv) {
	unsigned char R, G, B;
	hsvtorgb(&R, &G, &B, (unsigned char)hsv.val[0], (unsigned char)hsv.val[1], (unsigned char)hsv.val[2]);
	return CV_RGB(R, G, B);
}

void drawBadge(CvArr *img, char *string, CvScalar fontColor, double fontSize, CvPoint badgeCenter, CvScalar badgeColor) {
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, fontSize, fontSize, 0, 2, 8);

	CvSize textSize;
	cvGetTextSize(string, &font, &textSize, NULL);

	CvPoint textPoint = cvPoint(badgeCenter.x - textSize.width / 2, badgeCenter.y + textSize.height / 2);
	double textWidthOrHeightMax = (textSize.height > textSize.width) ? textSize.height : textSize.width;

	cvCircle(img, badgeCenter, 2 + textWidthOrHeightMax, CV_RGB(255, 255, 255), CV_FILLED, 8, 0);
	cvCircle(img, badgeCenter, textWidthOrHeightMax, badgeColor, CV_FILLED, 8, 0);

	cvPutText(img, string, textPoint, &font, fontColor);
}

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
