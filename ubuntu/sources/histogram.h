#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <sys/types.h>
#include <opencv2/core/core_c.h>

#include "common.h"

DIP_EXTERN size_t calcularHistograma(IplImage *src, size_t *binsCount, size_t **bins);

DIP_EXTERN void graficarHistograma(IplImage *dst, size_t binsCount, size_t *bins);

#endif /* HISTOGRAM_H */