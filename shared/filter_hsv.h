#ifndef FILTER_HSV_H
#define FILTER_HSV_H

#include "common.h"

DIP_EXTERN_BEGIN

int maskByHSV(IplImage *src, IplImage *dst, CvScalar minHSV, CvScalar maxHSV);

int filterByHSV(IplImage *src, IplImage *dst, CvScalar minHSV, CvScalar maxHSV);

DIP_EXTERN_END

#endif /* FILTER_HSV_H */
