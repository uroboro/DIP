#ifndef FACE_DETECTION_H
#define FACE_DETECTION_H

#include <opencv2/core/core_c.h>

#include "common.h"

DIP_EXTERN_BEGIN

int findFaces(CvArr* src, CvRect **rects, double scale, double *t);

void drawFaces(CvArr *dst, size_t facesCount, CvRect *faces);

void doFaces(IplImage *src, IplImage *dst, double scale);

DIP_EXTERN_END

#endif /* FACE_DETECTION_H */
