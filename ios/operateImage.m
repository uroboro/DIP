#include "messages.h"

#include "operateImage.h"
#include "UIImage+IplImage.h"

#include <opencv2/imgproc/imgproc_c.h>

#include "ocv_cuantify.h"
#include "ocv_samplify.h"
#include "ocv_histogram.h"
#include "ocv_histogramac.h"

CGImageRef operateImageRefCreate(CGImageRef imageRef0, CGImageRef imageRef1, NSMutableDictionary *options) {
	if (!imageRef0) { present(1, "!imageRef0"); return nil; }
	//if (!imageRef1) { present(1, "!imageRef1"); return nil; }

	IplImage *iplImage = IplImageFromCGImage(imageRef0);
	IplImage *iplImage1 = IplImageFromCGImage(imageRef1);

	IplImage *iplImageOut = NULL;

	if ([(NSString *)options[@"mode"] isEqualToString:@"double"]) {
		//iplImageOut = ocv_subtract(iplImage, iplImage1);
	} else {
		unsigned int operation = [options[@"operation"] intValue];
		unsigned int samples = [options[@"samples"] intValue];
		unsigned int colorDepth = [options[@"colorDepth"] intValue];
		unsigned int kernelSize = [options[@"kernelSize"] intValue];

		unsigned int binThreshold = 192;
		unsigned int lThreshold = 50;
		unsigned int hThreshold = 200;
		switch (operation) {
		case 1:
			iplImageOut = ocv_samplify(iplImage, samples);
			break;

		case 2:
			iplImageOut = ocv_cuantify(iplImage, colorDepth);
			break;

		case 3:
			{
			IplImage *grayscale = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 1);
			cvCvtColor(iplImage, grayscale, CV_RGB2GRAY);
			iplImageOut = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 3);
			cvMerge(grayscale, grayscale, grayscale, NULL, iplImageOut);
			cvReleaseImage(&grayscale);
			}
			break;

		case 4: // Invert
			{
			IplImage *iplImageTmp1 = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 3);
			cvSet(iplImageTmp1, cvScalarAll(255), NULL);
			cvSub(iplImageTmp1, iplImage, iplImageTmp1, NULL);

			iplImageOut = iplImageTmp1;
			}
			break;

		case 5:
			iplImageOut = ocv_histogram(iplImage);
			break;

		case 6:
			iplImageOut = ocv_histogramac(iplImage);
			break;

		case 7: // Gaussian
			{
			IplImage *iplImageTmp2 = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 3);
			cvSmooth(iplImage, iplImageTmp2, CV_GAUSSIAN, kernelSize, 0, 0, 0);

			iplImageOut = iplImageTmp2;
			}
			break;

		case 8: // LoG
			{
			IplImage *logMask = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 1);
			cvCvtColor(iplImage, logMask, CV_RGB2GRAY);
			cvSmooth(logMask, logMask, CV_GAUSSIAN, kernelSize + 2, 0, 0, 0);
			cvLaplace(logMask, logMask, kernelSize);

			iplImageOut = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 3);
			cvMerge(logMask, logMask, logMask, NULL, iplImageOut);
			cvReleaseImage(&logMask);
			}
			break;

		case 9: // LoG as mask
			{
			IplImage *logMask = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 1);
			cvCvtColor(iplImage, logMask, CV_RGB2GRAY);
			cvSmooth(logMask, logMask, CV_GAUSSIAN, kernelSize + 2, 0, 0, 0);
			cvLaplace(logMask, logMask, kernelSize);
			cvThreshold(logMask, logMask, binThreshold, 1, CV_THRESH_BINARY_INV);
			IplImage *iplImageMasked = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 3);
			cvCopy(iplImage, iplImageMasked, logMask);

			iplImageOut = iplImageMasked;
			}
			break;

		case 10: // Canny
			{
			IplImage *cannyMask = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 1);
			cvCvtColor(iplImage, cannyMask, CV_RGB2GRAY);
			cvSmooth(cannyMask, cannyMask, CV_GAUSSIAN, kernelSize + 2, 0, 0, 0);
			cvCanny(cannyMask, cannyMask, lThreshold, hThreshold, 3);

			iplImageOut = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 3);
			cvMerge(cannyMask, cannyMask, cannyMask, NULL, iplImageOut);
			cvReleaseImage(&cannyMask);
			}
			break;

		case 11: // Canny as mask
			{
			IplImage *cannyMask = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 1);
			cvCvtColor(iplImage, cannyMask, CV_RGB2GRAY);
			cvSmooth(cannyMask, cannyMask, CV_GAUSSIAN, kernelSize + 2, 0, 0, 0);
			cvCanny(cannyMask, cannyMask, lThreshold, hThreshold, 3);
			cvThreshold(cannyMask, cannyMask, binThreshold, 1, CV_THRESH_BINARY_INV);
			IplImage *iplImageMasked = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 3);
			cvCopy(iplImage, iplImageMasked, cannyMask);
			cvReleaseImage(&cannyMask);

			iplImageOut = iplImageMasked;
			}
			break;

		case 12: // Hue
			{
			IplImage *tmp3d = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 3);
			IplImage *tmp1d = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 1);

			cvCvtColor(iplImage, tmp3d, CV_RGB2HSV);
			cvSplit(tmp3d, tmp1d, NULL, NULL, NULL);

			cvSmooth(tmp1d, tmp1d, CV_GAUSSIAN, kernelSize + 2, 0, 0, 0);

			cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
			cvReleaseImage(&tmp1d);

			iplImageOut = tmp3d;
			}
			break;

		case 13: // Hue + Otsu
			{
			IplImage *tmp3d = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 3);
			IplImage *tmp1d = cvCreateImage(cvGetSize(iplImage), IPL_DEPTH_8U, 1);

			cvCvtColor(iplImage, tmp3d, CV_RGB2HSV);
			cvSplit(tmp3d, tmp1d, NULL, NULL, NULL);

			cvSmooth(tmp1d, tmp1d, CV_GAUSSIAN, kernelSize + 2, 0, 0, 0);

			cvThreshold(tmp1d, tmp1d, 0, 255, CV_THRESH_OTSU);

			cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
			cvReleaseImage(&tmp1d);

			iplImageOut = tmp3d;
			}
			break;

		default:
			iplImageOut = cvCloneImage(iplImage);
			break;
		}
	}

	cvReleaseImage(&iplImage);
	cvReleaseImage(&iplImage1);

	CGImageRef imageRef = CGImageFromIplImage(iplImageOut);
	cvReleaseImage(&iplImageOut);

	return imageRef;
}


UIImage *operateImageCreate(UIImage *image0, UIImage *image1, NSMutableDictionary *options) {
	CGImageRef imageRef0 = image0.CGImage;
	CGImageRef imageRef1 = image1.CGImage;

	CGImageRef imageRefOut = operateImageRefCreate(imageRef0, imageRef1, options);

	UIImage *uiImage = [[UIImage alloc] initWithCGImage:imageRefOut];
	CGImageRelease(imageRefOut);

	return uiImage;
}