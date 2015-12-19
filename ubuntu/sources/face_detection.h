#ifndef FACE_DETECTION_H
#define FACE_DETECTION_H

#include <opencv2/core/core_c.h>

int findFaces(CvArr* src, CvRect **rects, double scale, double *t);
void drawFaces(CvArr *dst, size_t facesCount, CvRect *faces);

#endif /* FACE_DETECTION_H */
