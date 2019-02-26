#include "filter_grayscale.h"
#include <math.h>

#if DIP_DARWIN
#include <dispatch/dispatch.h>
#endif

int maskByDistance2Grayscale(IplImage *src, IplImage *dst, int minDistance) {
	IplImage *tmp3d = cvCreateImage(cvGetSize(src), src->depth, 3);
	ocvDistance2Grayscale(src, tmp3d);
	cvSmooth(tmp3d, tmp3d, CV_GAUSSIAN, 9, 0, 0, 0);
	cvCvtColor(tmp3d, dst, CV_RGB2GRAY);
	cvReleaseImage(&tmp3d);
	#if DIP_DESKTOP
	cvEqualizeHist(dst, dst);
	#endif
	//CVSHOW("grayscale2", dst->width*2/3, dst->height*2/3, dst->width/2, dst->height/2, dst);
	#if DIP_MOBILE
	cvThreshold(dst, dst, minDistance, 255, CV_THRESH_BINARY);
	#endif
	return 0;
}

int ocvDistance2Grayscale(IplImage *src, IplImage *dst) {
	if (!src || !dst) { /*present(1, "!image"); */return 1; }

	if (src->nChannels == 3) {
		#if DIP_DARWIN
			dispatch_group_t g = dispatch_group_create();
			dispatch_queue_t q = dispatch_queue_create("com.uroboro.operator.gray", DISPATCH_QUEUE_CONCURRENT);
		#endif
		for (unsigned int y = 0; y < src->height; y++) {
			#if DIP_DARWIN
				dispatch_group_async(g, q, ^{
			#endif
			for (unsigned int x = 0; x < src->width; x++) {
				CvScalar p = cvGet2D(src, y, x);
				// Calculate distance to grayscale value
				cvSet2D(dst, y, x, cvScalarAll(sqrt(2.0 / 3 * (
					p.val[0] * (p.val[0] - p.val[1])
					+ p.val[1] * (p.val[1] - p.val[2])
					+ p.val[2] * (p.val[2] - p.val[0])
				))));
			}
			#if DIP_DARWIN
				});
			#endif
		}
		#if DIP_DARWIN
			dispatch_group_wait(g, DISPATCH_TIME_FOREVER);
			dispatch_release(q);
		#endif
	}

	return 0;
}
