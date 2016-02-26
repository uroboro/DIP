#ifndef HOUGH_LINES_H
#define HOUGH_LINES_H

#include <opencv2/core/core_c.h>

#include "common.h"

DIP_EXTERN_BEGIN

void houghLines(IplImage *src, IplImage *dst);

DIP_EXTERN_END

#endif /* HOUGH_LINES_H */
