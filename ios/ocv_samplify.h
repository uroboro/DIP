#ifndef OCV_SAMPLIFY_H
#define OCV_SAMPLIFY_H

#include <opencv2/core/core_c.h>

#include "common.h"

DIP_EXTERN IplImage *ocv_samplify(IplImage *image, unsigned int sampleSize);

#endif /* OCV_SAMPLIFY_H */
