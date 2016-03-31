#ifndef _FILTER_GRAYSCALE_H
#define _FILTER_GRAYSCALE_H

#include "common.h"

DIP_EXTERN_BEGIN

void ocvDistance2GrayscaleMat(cv::Mat& src, cv::Mat& dst);

void maskByDistance2GrayscaleMat(cv::Mat& src, cv::Mat& dst, int minDistance);

DIP_EXTERN_END

#endif /* FILTER_GRAYSCALE_H */
