#ifndef FIXES_H
#define FIXES_H

#include "common.h"

#define cvReleaseMemStorage2(sequence) if (sequence && sequence->storage) { cvReleaseMemStorage(&sequence->storage); }

DIP_EXTERN_BEGIN

void cvCopy2(CvArr *src, CvArr *dst, CvArr *mask);

void cvCopyNonZero(CvArr *src, CvArr *dst, CvArr *mask);

int cvFindContours2(IplImage* image, CvMemStorage* storage, CvSeq** first_contour, int header_size, int mode, int method, CvPoint offset);

void cvTranslateImage2(IplImage *src, IplImage *dst, int offsetx, int offsety);

int cvContourArea2(CvSeq *contour);

IplImage* createSubArray(IplImage *src, CvRect rect);

void cvRectangle2(CvArr* img, CvRect rect, CvScalar color, int thickness, int line_type, int shift);

void cvFillConvexPoly2(CvArr* img, CvSeq *points, CvScalar color, int line_type, int shift);

void cvBox2(CvArr* img, CvBox2D rect, CvScalar color, int thickness, int line_type, int shift);

void ocvResizeFrame(IplImage *src, IplImage *dst);

void ocv2DAffineMatrix(CvMat* map_matrix, CvPoint2D32f c, float a);

void cvClose(CvArr *src, CvArr *dst, CvArr *mask, size_t n);

DIP_EXTERN_END

#endif /* FIXES_H */
