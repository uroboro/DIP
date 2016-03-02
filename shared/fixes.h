#ifndef FIXES_H
#define FIXES_H

#include "common.h"

DIP_EXTERN void cvCopy2(CvArr *src, CvArr *dst, CvArr *mask);

DIP_EXTERN void cvCopyNonZero(CvArr *src, CvArr *dst, CvArr *mask);

DIP_EXTERN int cvFindContours2(IplImage* image, CvMemStorage* storage, CvSeq** first_contour, int header_size, int mode, int method, CvPoint offset);

DIP_EXTERN void cvTranslateImage2(IplImage *src, IplImage *dst, int offsetx, int offsety);

DIP_EXTERN int cvContourArea2(CvSeq *contour);

DIP_EXTERN IplImage* createSubArray(IplImage *src, CvRect rect);

DIP_EXTERN void cvRectangle2(CvArr* img, CvRect rect, CvScalar color, int thickness, int line_type, int shift);

DIP_EXTERN void cvFillConvexPoly2(CvArr* img, CvSeq *points, CvScalar color, int line_type, int shift);

DIP_EXTERN void cvBox2(CvArr* img, CvBox2D rect, CvScalar color, int thickness, int line_type, int shift);

DIP_EXTERN void ocvResizeFrame(IplImage *src, IplImage *dst);

DIP_EXTERN void ocv2DAffineMatrix(CvMat* map_matrix, CvPoint2D32f c, float a);

DIP_EXTERN void cvClose(CvArr *src, CvArr *dst, CvArr *mask, size_t n);

#endif /* FIXES_H */
