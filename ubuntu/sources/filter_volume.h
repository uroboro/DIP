#ifndef FILTER_VOLUME_H
#define FILTER_VOLUME_H

#include <opencv2/core/core_c.h>

int filterByVolume(IplImage *src, IplImage *dst, long minVolume);

#endif /* FILTER_VOLUME_H */
