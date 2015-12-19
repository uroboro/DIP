#ifndef FILTER_HSV_H
#define FILTER_HSV_H

#include <opencv2/core/core_c.h>

int filterByHSV(IplImage *src, CvScalar minHSV, CvScalar maxHSV, IplImage *dst);

#endif /* FILTER_HSV_H */
