#ifndef FILTER_VOLUME_H
#define FILTER_VOLUME_H

#include "common.h"

DIP_EXTERN int filterByVolume(IplImage *src, IplImage *dst, long minVolume);

#endif /* FILTER_VOLUME_H */
