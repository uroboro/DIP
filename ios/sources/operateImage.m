#include "messages.h"

#include "operateImage.h"
#include "UIImage+IplImage.h"

#include "fixes.h"
#include "fixes.hpp"

#include "ocv_hand.h"
#include "drawing.h"

#include "try.h"
#import "utils.h"

#define OCV_GRAYSCALE_DISTANCE 20

void ocv_handAnalysis(IplImage *src, IplImage *dst) {
	double _time = (double)getTickCount();

	IplImage *tmp3d = cvCreateImage(cvGetSize(src), src->depth, 3);
	cvCopy(src, tmp3d,  NULL);

	IplImage *tmp1d = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 1);
	cvSet(tmp1d, cvScalarAll(255), NULL);

	IplImage *red3d = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 3);

	// Background subtraction
	#if 0
		static IplImage *background = NULL;
		static int setupFrameCount = 60;
		//present(0, "%s", ((NSString *)options[@"inputType"]).UTF8String);
		if (setupFrameCount) {
			setupFrameCount--;
			if ([options[@"inputType"] isEqualToString:@"video"]) {
				static IplImage *backgroundAcc = NULL;
				if (cvGetSize(tmp3d).width < cvGetSize(tmp3d).height) {
				DO_ONCE(
					background = cvCloneImage(tmp3d);
					backgroundAcc = cvCreateImage(cvGetSize(background), IPL_DEPTH_32F, background->nChannels);
					cvSet(backgroundAcc, cvScalarAll(0), NULL);
					cvRunningAvg(background, backgroundAcc, 1, NULL);
				)
				TRY_ONCE(
					cvRunningAvg(background, backgroundAcc, OCV_ACCUMULATOR_ALPHA, NULL);
					cvConvertScale(backgroundAcc, background, 1, 0);
				)
				}
			}
		} else {
			cvAbsDiff(background, tmp3d, tmp3d);
			cvCvtColor(tmp3d, tmp1d, CV_RGB2GRAY);
			cvThreshold(tmp1d, tmp1d, (int)(255 * floatingValue), 255, CV_THRESH_BINARY);
			//cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
			cvCopy2(iplImage, tmp3d, tmp1d);
		}
		if (0&&[options[@"inputType"] isEqualToString:@"image"]) {
			cvCvtColor(tmp3d, tmp1d, CV_BGR2GRAY);
			cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
		}

		//goto end;
	#endif

	cvCvtColor(tmp3d, tmp3d, CV_RGB2BGR);

	CvScalar minScalar = cvScalar(160, 30, 50, 0);
	CvScalar maxScalar = cvScalar(30, 255, 255, 255);
	TRY_ONCE(
	ocvPrefilterImageMask(tmp3d, tmp1d, OCV_GRAYSCALE_DISTANCE, minScalar, maxScalar);
	)

	if (0) { // Show masked image
		if (0) { // Show masked image
			NSLog2("copy masked");
			cvCopy2(tmp3d, tmp3d, tmp1d);
		} else { // Show mask
			NSLog2("copy mask");
			cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
		}
		cvCvtColor(tmp3d, tmp3d, CV_BGR2RGB);
	} else { // Show original
		NSLog2("use original");
		cvCopy(src, tmp3d,  NULL);
	}
	//goto end;
	{ // Add red alpha layer to show ignored areas
		IplImage *red1d = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 1);
		cvNot(tmp1d, red1d);
		cvMerge(red1d, NULL, NULL, NULL, red3d);
		cvReleaseImage(&red1d);
	}
	//goto end;

	NSLog2("get contours");
	CvSeq *contourSeq = NULL;
	cvFindContours2(tmp1d, cvCreateMemStorage(0), &contourSeq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	// Iterate over each contour
	int contourCount = 0; for (CvSeq* seq = contourSeq; seq != 0; seq = seq->h_next) contourCount++;
	NSLog2(([NSString stringWithFormat:@"foreach contour (%d)", contourCount]).UTF8String);
	if (contourCount > 0) {
		for (CvSeq* seq = contourSeq; seq != 0; seq = seq->h_next) {
			IplImage *overlay = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 3);

			__block ocvHand myHand;

			NSLog2("analize contour");
			__block int ret = 0;
			TRY_ONCE(
			ret = ocvAnalizeContour(seq, overlay, &myHand);
			)

			// Copy overlay and text to output image
			if (ret && (myHand.fingers > 2 && myHand.fingers < 6)) {
				NSLog2("draw hand");
				//ocvDrawHandInfo(overlay, myHand);
				char *handPaths[] = { "1456448219_icon_2_rock_n_roll.png", "1456448230_icon_3_high_five.png" };
				UIImage *uiImage = [[UIImage alloc] initWithContentsOfFile:UtilsResourcePathWithName(@(handPaths[(myHand.fingers == 5)]))];
				IplImage *sprite = IplImageFromCGImage(uiImage.CGImage);
				[uiImage release];

				ocvCreateHandIconWithHand(overlay, sprite, myHand);
				cvReleaseImage(&sprite);
				cvCopyNonZero(overlay, tmp3d, NULL);
			} else {
				cvDrawContours(red3d, seq, CV_RGB(0,0,255), CV_RGB(0,0,255), 0, CV_FILLED, 8, cvPoint(0, 0));
			}
			cvReleaseImage(&overlay);
		}
		cvReleaseMemStorage(&contourSeq->storage);
	}

	goto end;
	end:;
	cvAddWeighted(tmp3d, 0.7, red3d, 0.3, 0, tmp3d);

	NSLog2("end proc");
	cvReleaseImage(&tmp1d);
	cvReleaseImage(&red3d);

	{ // On-screen debug data
		_time = ((double)getTickCount() - _time) / getTickFrequency();
		CvFont font; double fontSize = 1;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, fontSize, fontSize, 0, 2, 8);
		{
			char buf[32]; sprintf(buf, "time: %dms (%.1f fps)?", (int)(1000 * _time), 1/_time);
			cvPutText(tmp3d, buf, cvPoint(10, tmp3d->height - 40), &font, CV_RGB(0, 255, 0));
		}
		{
			char buf[32]; sprintf(buf, "size: %dx%d", tmp3d->width, tmp3d->height);
			cvPutText(tmp3d, buf, cvPoint(10, tmp3d->height - 10), &font, CV_RGB(0, 255, 0));
		}
	}
	cvCopy(tmp3d, dst, NULL);
	cvReleaseImage(&tmp3d);
}

CGImageRef operateImageRefCreate(CGImageRef imageRef0, CGImageRef imageRef1, NSMutableDictionary *options) {
	if (!imageRef0) { present(1, "!imageRef0"); return nil; }
	NSLog2("operating");

	#define SCALE 0
	#if SCALE
		float floatingValue = options[@"floatingValue"] ? ((NSNumber *)options[@"floatingValue"]).floatValue : 1;
		NSLog2(([NSString stringWithFormat:@"floatingValue %@", options[@"floatingValue"]]).UTF8String);
		// Scale down input image
		if (floatingValue < 1) { CGImageRef tmp = CreateScaledCGImageFromCGImage(imageRef0, floatingValue); if (tmp) { imageRef0 = tmp; } }
	#endif

	IplImage *iplInput = IplImageFromCGImage(imageRef0);
	if (!iplInput) { present(1, "!iplInput"); return nil; }

	IplImage *iplOutput = cvCloneImage(iplInput);

	//test orientation handling
	//if ([options[@"inputType"] isEqualToString:@"image"]) { cvFlip(iplImage, NULL, 1); }

	ocv_handAnalysis(iplInput, iplOutput);
	cvReleaseImage(&iplInput);

	CGImageRef imageRefOut = CGImageFromIplImage(iplOutput);
	cvReleaseImage(&iplOutput);

	#if SCALE
		// Scale up output image
		if (floatingValue < 1) { CGImageRef tmp = CreateScaledCGImageFromCGImage(imageRefOut, 1/floatingValue); if (tmp) { CGImageRelease(imageRefOut); imageRefOut = tmp; } }
	#endif

	options[@"fps"] = @(5);

	return imageRefOut;
}

UIImage *operateImageCreate(UIImage *image0, UIImage *image1, NSMutableDictionary *options) {
	CGImageRef imageRef0 = image0.CGImage;
	CGImageRef imageRef1 = image1.CGImage;

	CGImageRef imageRefOut = operateImageRefCreate(imageRef0, imageRef1, options);

	UIImage *uiImage = [[UIImage alloc] initWithCGImage:imageRefOut];
	CGImageRelease(imageRefOut);

	return uiImage;
}
