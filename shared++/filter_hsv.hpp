#ifndef _FILTER_HSV_H
#define _FILTER_HSV_H

#include "common.h"

DIP_EXTERN_BEGIN

void maskByHSVMat(cv::Mat& src, cv::Mat& dst, cv::Scalar minHSV, cv::Scalar maxHSV);

void filterByHSVMat(cv::Mat& src, cv::Mat& dst, cv::Scalar minHSV, cv::Scalar maxHSV);

DIP_EXTERN_END

#endif /* _FILTER_HSV_H */
