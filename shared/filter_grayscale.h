#ifndef FILTER_GRAYSCALE_H
#define FILTER_GRAYSCALE_H

#include "common.h"

DIP_EXTERN_BEGIN

int maskByDistance2Grayscale(IplImage *src, IplImage *dst, int minDistance);

int ocvDistance2Grayscale(IplImage *src, IplImage *dst);

DIP_EXTERN_END

#endif /* FILTER_GRAYSCALE_H */
