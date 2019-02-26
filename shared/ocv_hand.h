#ifndef OCV_HAND
#define OCV_HAND

#include "common.h"

DIP_EXTERN_BEGIN

typedef struct ocvHand {
	int fingers;
	int orientation; // 1 left, 0 right
	float controlAngle; // index-to-thumb angle
	float size; // palm/area to search when making persistence
	CvPoint center;
	CvPoint thumbTip;
	CvPoint indexTip;
} ocvHand;

void ocvCreateHandIconWithHand(IplImage *layer, IplImage *sprite, ocvHand myHand);

void ocvPrefilterImageMask(IplImage *src, IplImage *dst, int grayscaleDistance, CvScalar minScalar, CvScalar maxScalar);

int ocvAnalizeContour(uint32_t idx, CvSeq *seq, IplImage *overlay, ocvHand *myHand);

void ocvDrawHandInfo(IplImage *overlay, ocvHand myHand);

void ocv_handAnalysis(IplImage *src, IplImage *dst);

#ifdef __cplusplus
void ocv_handAnalysisMat(cv::Mat& src, cv::Mat& dst);
#endif

DIP_EXTERN_END

#endif /* OCV_HAND */
