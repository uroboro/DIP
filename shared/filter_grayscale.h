#ifndef FILTER_GRAYSCALE_H
#define FILTER_GRAYSCALE_H

#include <opencv2/core/core_c.h>

#include "common.h"

DIP_EXTERN int maskByDistance2Grayscale(IplImage *src, int minDistance, IplImage *dst);

DIP_EXTERN int ocvDistance2Grayscale(IplImage *src, IplImage *dst);

#endif /* FILTER_GRAYSCALE_H */
