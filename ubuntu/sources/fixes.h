#ifndef FIXES_H
#define FIXES_H

#include <opencv2/core/core_c.h>

void cvCopy2(CvArr *src, CvArr *dst, CvArr *mask);

int cvFindContours2(IplImage* image, CvMemStorage* storage, CvSeq** first_contour, int header_size, int mode, int method, CvPoint offset);

void cvTranslateImage2(IplImage *src, IplImage *dst, int offsetx, int offsety);

int cvContourArea2(CvSeq *contour);

IplImage* createSubArray(IplImage *src, CvRect rect);

#endif /* FIXES_H */
