#ifndef FIXES_H
#define FIXES_H

#include <opencv2/core/core_c.h>

#include "common.h"

DIP_EXTERN void cvCopy2(CvArr *src, CvArr *dst, CvArr *mask);

DIP_EXTERN int cvFindContours2(IplImage* image, CvMemStorage* storage, CvSeq** first_contour, int header_size, int mode, int method, CvPoint offset);

DIP_EXTERN void cvTranslateImage2(IplImage *src, IplImage *dst, int offsetx, int offsety);

DIP_EXTERN int cvContourArea2(CvSeq *contour);

DIP_EXTERN IplImage* createSubArray(IplImage *src, CvRect rect);

#endif /* FIXES_H */
