#ifndef FILTER_HSV_H
#define FILTER_HSV_H

#include <opencv2/core/core_c.h>

#include "common.h"

DIP_EXTERN int maskByHSV(IplImage *src, CvScalar minHSV, CvScalar maxHSV, IplImage *dst);

DIP_EXTERN int filterByHSV(IplImage *src, CvScalar minHSV, CvScalar maxHSV, IplImage *dst);

#endif /* FILTER_HSV_H */
