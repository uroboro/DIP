#ifndef FIXES_H
#define FIXES_H

#include <opencv2/core/core_c.h>

void cvCopy2(CvArr *src, CvArr *dst, CvArr *mask);
IplImage* createSubArray(IplImage *src, CvRect rect);

#endif /* FIXES_H */
