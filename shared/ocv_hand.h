#ifndef OCV_HAND
#define OCV_HAND

#include <opencv2/core/core_c.h>

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

DIP_EXTERN_END

#endif /* OCV_HAND */
