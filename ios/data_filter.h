#ifndef DATA_FILTER_H
#define DATA_FILTER_H

#define CLAMP_(p, under, min, over, max) \
({ __typeof(p) p_ = p; (p_ < (under)) ? (min) : (p_ >= (over)) ? (max) : p_; })

#define CLAMP(p, min, max) CLAMP_(p, min, min, max, max)

#define CLAMP_TYPE(p, type) CLAMP(p, 0, 2 << (8 * sizeof(type)))

#define CLAMP_PIXEL(p) CLAMP_TYPE(p, unsigned char)

typedef enum ExceptionType {
	ETEcho,
	ETMirror,
	ETToroid
} ExceptionType;

#include "common.h"

DIP_EXTERN unsigned char data_pixelFromImageAtCoordinateWithExceptionType(unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep,
	long int x, long int y, ExceptionType exceptionType);

DIP_EXTERN unsigned char *data_filter(unsigned char **dst, unsigned char *src,
	unsigned int width, unsigned int height, unsigned int widthStep,
	ExceptionType exceptionType, double *kernel, unsigned int kernelSize);

#endif /* DATA_FILTER_H */
