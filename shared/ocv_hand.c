#include "ocv_hand.h"
#include <opencv2/imgproc/imgproc_c.h>

#include "geometry.h"
#include "fixes.h"

void ocvCreateHandIconWithHand(IplImage *layer, IplImage *sprite, ocvHand myHand) {
	CvSize spriteSize = cvGetSize(sprite);

	if (myHand.orientation) {
		cvFlip(sprite, NULL, 1);
	}

	{
		IplImage *tmp1d = cvCreateImage(cvGetSize(sprite), sprite->depth, 1);
		cvCvtColor(sprite, tmp1d, CV_BGR2GRAY);
		cvNot(tmp1d, tmp1d);
		cvMerge(tmp1d, tmp1d, tmp1d, NULL, sprite);
		cvThreshold(tmp1d, tmp1d, 2, 255, CV_THRESH_BINARY);
		cvReleaseImage(&tmp1d);
	}

	IplImage *tmp3d = cvCreateImage(cvGetSize(layer), layer->depth, layer->nChannels);
	ocvResizeFrame(sprite, tmp3d);

	cvTranslateImage2(tmp3d, tmp3d, myHand.center.x - spriteSize.width / 2, myHand.center.y - spriteSize.height / 2);

	float phaseI = M_PI / 2 + cvPointPhase(cvPointSubtract(myHand.indexTip, myHand.center));
	CvMat* map_matrix = cvCreateMat(2, 3, CV_32FC1);
	ocv2DAffineMatrix(map_matrix, cvPointTo32f(myHand.center), phaseI);
	cvWarpAffine(tmp3d, tmp3d, map_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, cvScalarAll(0));

	cvCopy(tmp3d, layer, NULL);
	cvReleaseImage(&tmp3d);
}
