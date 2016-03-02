#ifndef OCV_HAND
#define OCV_HAND

#include "common.h"

DIP_EXTERN_BEGIN

typedef struct ocvHand {
	int fingers;
	int orientation; // 1 left, 0 right
	float controlAngle; // index-to-thumb angle
	CvPoint center;
	CvPoint thumbTip;
	CvPoint indexTip;
} ocvHand;

void ocvCreateHandIconWithHand(IplImage *layer, IplImage *sprite, ocvHand myHand);

void ocvPrefilterImageMask(CvArr *src, IplImage *dst, int grayscaleDistance, CvScalar minScalar, CvScalar maxScalar);

int ocvAnalizeContour(CvSeq *seq, IplImage *overlay, ocvHand *myHand);

void ocvDrawHandInfo(IplImage *overlay, ocvHand myHand);

void ocv_handAnalysis(IplImage *src, IplImage *dst);

DIP_EXTERN_END

#endif /* OCV_HAND */
