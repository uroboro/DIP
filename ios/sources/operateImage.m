#include "messages.h"

#include "operateImage.h"
#include "UIImage+IplImage.h"

#include <opencv2/imgproc/imgproc_c.h>

#include "ocv_cuantify.h"
#include "ocv_samplify.h"
#include "ocv_histogram.h"
#include "ocv_histogramac.h"

#include "fixes.h"
#include "filter_volume.h"
#include "filter_hsv.h"
#include "filter_grayscale.h"

#include "try.h"

static void hsvtorgb(unsigned char *r, unsigned char *g, unsigned char *b, unsigned char h, unsigned char s, unsigned char v) {
	unsigned char region, fpart, p, q, t;

	if (s == 0) {
		/* color is grayscale */
		*r = *g = *b = v;
		return;
	}

	/* make hue 0-5 */
	region = h / 43;
	/* find remainder part, make it from 0-255 */
	fpart = (h - (region * 43)) * 6;

	/* calculate temp vars, doing integer multiplication */
	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * fpart) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;

	/* assign temp vars based on color cone region */
	switch(region) {
		case 0:
			*r = v; *g = t; *b = p; break;
		case 1:
			*r = q; *g = v; *b = p; break;
		case 2:
			*r = p; *g = v; *b = t; break;
		case 3:
			*r = p; *g = q; *b = v; break;
		case 4:
			*r = t; *g = p; *b = v; break;
		default:
			*r = v; *g = p; *b = q; break;
	}

	return;
}

static CvScalar cvScalarRGBFromHSV(CvScalar hsv) {
	unsigned char R, G, B;
	hsvtorgb(&R, &G, &B, (unsigned char)hsv.val[0], (unsigned char)hsv.val[1], (unsigned char)hsv.val[2]);
	return CV_RGB(R, G, B);
}

void drawBadge(CvArr *img, char *string, CvScalar fontColor, double fontSize, CvPoint badgeCenter, CvScalar badgeColor) {
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, fontSize, fontSize, 0, 2, 8);

	CvSize textSize;
	cvGetTextSize(string, &font, &textSize, NULL);

	CvPoint textPoint = cvPoint(badgeCenter.x - textSize.width / 2, badgeCenter.y + textSize.height / 2);
	double textWidthOrHeightMax = (textSize.height > textSize.width) ? textSize.height : textSize.width;

	cvCircle(img, badgeCenter, 2 + textWidthOrHeightMax, CV_RGB(255, 255, 255), CV_FILLED, 8, 0);
	cvCircle(img, badgeCenter, textWidthOrHeightMax, badgeColor, CV_FILLED, 8, 0);

	cvPutText(img, string, textPoint, &font, fontColor);
}

void cvClose(CvArr *src, CvArr *dst, CvArr *mask, size_t n) {
	cvCopy(src, dst, mask);
	//for (size_t i = 0; i < n; i++) {
		cvErode(dst, dst, NULL, n);
		cvDilate(dst, dst, NULL, n);
	//}
}

CGImageRef CreateScaledCGImageFromCGImage(CGImageRef image, float scale) {
	// Get image width, height. We'll use the entire image.
	int width = CGImageGetWidth(image) * scale;
	int height = CGImageGetHeight(image) * scale;

	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	int bitmapBytesPerRow   = (width * 4);
	int bitmapByteCount     = (bitmapBytesPerRow * height);

	// Allocate memory for image data. This is the destination in memory
	// where any drawing to the bitmap context will be rendered.
	void *bitmapData = malloc(bitmapByteCount);
	if (bitmapData == NULL) {
		return nil;
	}

	// Create the bitmap context. We want pre-multiplied ARGB, 8-bits
	// per component. Regardless of what the source image format is
	// (CMYK, Grayscale, and so on) it will be converted over to the format
	// specified here by CGBitmapContextCreate.
	CGColorSpaceRef colorspace = CGImageGetColorSpace(image);
	CGContextRef context = CGBitmapContextCreate(bitmapData, width, height, 8,
		bitmapBytesPerRow, colorspace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
	CGColorSpaceRelease(colorspace);

	if (context == NULL)
		// error creating context
		return nil;

	// Draw the image to the bitmap context. Once we draw, the memory
	// allocated for the context for rendering will then contain the
	// raw image data in the specified color space.
	CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);

	CGImageRef imgRef = CGBitmapContextCreateImage(context);
	CGContextRelease(context);
	free(bitmapData);

	return imgRef;
}

#define NSLog2(message) NSLog(@"XXX Reached line \e[31m%d\e[0m, message: \e[32m%s\e[0m", __LINE__, message);

#define DO_ONCE(block) { static dispatch_once_t once ## __LINE__; dispatch_once(&once ## __LINE__, ^{block}); }


#define cvPointPolar(r, f)		cvPointFrom32f(cvPoint2D32f((r) * cos((f)), (r) * sin((f))))

#define cvPointScale(p, s)		cvPoint((s) * (p).x, (s) * (p).y)
#define cvPointModule(p)		sqrt(cvPointDot(p, p))
#define cvPointPhase(p) 		atan2((p).y, (p).x)

// p + q
#define cvPointAdd(p, q)		cvPoint((p).x + (q).x, (p).y + (q).y)
// p . q
#define cvPointDot(p, q)		((p).x * (q).x + (p).y * (q).y)
// p - q
#define cvPointSubtract(p, q)	cvPointAdd(p, cvPointScale(q, -1))

#define cvPointDistance(p, q)	cvPointModule(cvPointSubtract(p, q))
// (p + q) / 2
#define cvPointMidPoint(p, q)	cvPointScale(cvPointAdd(p, q), 0.5)
#define cvPointAngle(p, q)		acos(cvPointDot(p, q) / (cvPointModule(p) * cvPointModule(q)))
#define cvPointProject(p, q)	(cvPointDot(p, q) / cvPointModule(q)))

#define OCV_GRAYSCALE_DISTANCE 20
#define OCV_DEFECT_MIN_DEPTH 10
#define OCV_OBJECT_WIDTH_HEIGHT_RATIO 2
#define OCV_ACCUMULATOR_ALPHA 0.3


typedef struct ocvHand {
	CvPoint center;
	CvPoint thumbTip;
	CvPoint indexTip;
	int orientation; // 1 left, 0 right
	float angle; // index tip to center angle
	float controlAngle; // index-to-thumb angle
} ocvHand;

ocvHand myHand;

void ocvPrefilterImageMask(CvArr *src, IplImage *dst, int grayscaleDistance, CvScalar minScalar, CvScalar maxScalar) {
	IplImage *tmp1dg = cvCreateImage(cvGetSize(dst), dst->depth, 1);
	IplImage *tmp1dc = cvCreateImage(cvGetSize(dst), dst->depth, 1);

	{ // Distance from grayscale and threshold
		NSLog2("grayscale");
		maskByDistance2Grayscale(src, grayscaleDistance, tmp1dg);
	}

	{ // Filter out non skin tones
		NSLog2("skin");
		maskByHSV(src, minScalar, maxScalar, tmp1dc);
	}

	{
		IplImage *white = cvCloneImage(tmp1dc); cvSet(white, cvScalarAll(255), NULL);
		IplImage *tmp3d = cvCreateImage(cvGetSize(dst), dst->depth, 3);
		cvMerge(tmp1dg, white, tmp1dc, NULL, tmp3d);
		cvCvtColor(tmp3d, white, CV_BGR2GRAY);
		cvThreshold(white, white, 240, 255, CV_THRESH_BINARY);
		cvReleaseImage(&tmp3d);

		cvClose(white, white, NULL, 3);
		cvSmooth(white, white, CV_GAUSSIAN, 9, 0, 0, 0);
		cvThreshold(white, white, 127, 255, CV_THRESH_BINARY);
		filterByVolume(white, white, white->width * white->height / 200);

		//cvMerge(white, white, white, NULL, dst);
		cvCopy(white, dst, NULL);
		cvReleaseImage(&white);
		goto end;
	}

	// Stabilize output
	#if 0
		static IplImage *backgroundAcc = NULL;

		if (cvGetSize(tmp1d).width < cvGetSize(tmp1d).height) {
		DO_ONCE(
			backgroundAcc = cvCreateImage(cvGetSize(tmp1d), IPL_DEPTH_32F, tmp1d->nChannels);
			cvSet(backgroundAcc, cvScalarAll(0), NULL);
			cvRunningAvg(tmp1d, backgroundAcc, 1, NULL);
		)
		TRY_ONCE(
			cvRunningAvg(tmp1d, backgroundAcc, OCV_ACCUMULATOR_ALPHA, NULL);
			cvConvertScale(backgroundAcc, tmp1d, 1, 0);
		)
		cvThreshold(tmp1d, tmp1d, 200, 255, CV_THRESH_BINARY);
		}
	#endif

	// Analise each contour
	#if 0
		NSLog2("contour topology");
			CvSeq *contourSeq = NULL;
			cvFindContours(tmp1d, cvCreateMemStorage(0), &contourSeq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
			for (CvSeq* seq = contourSeq; seq != 0; seq = seq->h_next) {
				// Ignore contours with a ratio of w:h or h:w greater than OCV_OBJECT_WIDTH_HEIGHT_RATIO
				CvBox2D box = cvMinAreaRect2(seq, NULL);
				double seqRatio = box.size.width / box.size.height;

				double seqArea = cvContourArea(seq, CV_WHOLE_SEQ, 0);
				double screenArea = tmp1d->width * tmp1d->height;
				if (seqRatio > OCV_OBJECT_WIDTH_HEIGHT_RATIO || seqRatio < 1 / OCV_OBJECT_WIDTH_HEIGHT_RATIO || seqArea < 0.1 * screenArea) {
					//cvBox2(tmp1d, box, cvScalarAll(255), CV_FILLED, 8, 0);
					//cvEllipseBox(tmp1d, box, cvScalarAll(255), CV_FILLED, 8, 0);
					cvDrawContours(tmp1d, seq, cvScalarAll(0), cvScalarAll(0), 0, CV_FILLED, 8, cvPoint(0, 0));
				} else {
					cvDrawContours(tmp1d, seq, cvScalarAll(255), cvScalarAll(255), 0, CV_FILLED, 8, cvPoint(0, 0));
				}
			}
			//cvClearMemStorage(contourSeq->storage);
			cvReleaseMemStorage(&contourSeq->storage);
	#endif

	goto end;
	end:;
	cvReleaseImage(&tmp1dg);
	cvReleaseImage(&tmp1dc);
}

int ocvAnalizeContour(CvSeq *seq, IplImage *overlay, IplImage *textOverlay, ocvHand *myHand) {
	int r = 0;

	// Extra canvas to do single contour drawing
	IplImage *canvas = cvCreateImage(cvGetSize(overlay), overlay->depth, 1);
	cvDrawContours(canvas, seq, cvScalarAll(255), cvScalarAll(255), 0, CV_FILLED, 8, cvPoint(0, 0));

	// Defects
	// Assumed to be fingers (or wrist)
	NSLog2("get defects");
	CvSeq *defects = cvConvexityDefects(seq, cvConvexHull2(seq, NULL, CV_CLOCKWISE, 0), cvCreateMemStorage(0));
	if(01){ // Filter out the shallow ones
		NSLog2("deep defects");
		CvSeq* defectsSeq = cvCreateSeq(CV_SEQ_ELTYPE_PTR, sizeof(CvSeq), sizeof(CvConvexityDefect), cvCreateMemStorage(0));
		for (int i = 0; i < defects->total; i++) {
			CvConvexityDefect *defect = CV_GET_SEQ_ELEM(CvConvexityDefect, defects, i);
			if (defect->depth > OCV_DEFECT_MIN_DEPTH) {
				cvSeqPush(defectsSeq, defect);
			}
		}
		cvReleaseMemStorage(&defects->storage);
		defects = defectsSeq;
	}

	CvSeq* fingerTipSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), cvCreateMemStorage(0));
	CvSeq* pointSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), cvCreateMemStorage(0));

	NSLog2("get data");
	for (int i = 0; i < defects->total; i++) {
		CvConvexityDefect *defect = CV_GET_SEQ_ELEM(CvConvexityDefect, defects, i);
		cvSeqPush(pointSeq, defect->depth_point);

		CvConvexityDefect *prevDefect = CV_GET_SEQ_ELEM(CvConvexityDefect, defects, i - 1);
		CvPoint fingerTip = cvPointMidPoint(*defect->start, *prevDefect->end);
		cvSeqPush(fingerTipSeq, &fingerTip);
	}
	cvReleaseMemStorage(&defects->storage);

	// Only convex contours are valid
	// Work with 3 to 6 deep defects
	if (cvCheckContourConvexity(pointSeq) && (pointSeq->total >= 3 && pointSeq->total <= 6)) {
		r = 1;
		NSLog2("convex and 3-6 fingers");

		// Get circle enclosing defects
		// Not an excellent approximation of the palm, but close enough
		float radius = 0.0;
		CvPoint2D32f centerf;
		cvMinEnclosingCircle(pointSeq, &centerf, &radius);
		CvPoint center = cvPointFrom32f(centerf);
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		myHand->center = center;
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		{ // Place radius between shortest fingertip and longest deep defect
			NSLog2("radius setting");
			//find longest near radius
			float lnRadius = 0;
			for (int i = 0; i < pointSeq->total; i++) {
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, pointSeq, i);
				int distance = cvPointDistance(center, *point);
				if (distance > lnRadius) {
					lnRadius = distance;
				}
			}
			lnRadius += 2;

			//find shortest far radius
			float sfRadius = 1000;
			for (int i = 0; i < fingerTipSeq->total; i++) {
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, i);
				int distance = cvPointDistance(center, *point);
				if (distance < sfRadius) {
					sfRadius = distance;
				}
			}
			sfRadius -= 2;
			radius = (lnRadius + sfRadius) / 2;
		}

		// Find fingers
		CvSeq *circleContours = NULL;
		{ // Get contour of circle starting on black pixel
			NSLog2("circle");
			{ // Get circle points
				IplImage *tmp1d = cvCreateImage(cvGetSize(canvas), canvas->depth, 1);
				cvCircle(tmp1d, center, radius, cvScalarAll(255), CV_FILLED, 8, 0);
				cvFindContours(tmp1d, cvCreateMemStorage(0), &circleContours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
				cvReleaseImage(&tmp1d);
			}

			NSLog2("circle rotate");
			// Rotate sequence so that the first item contains a black pixel
			int safeCount = circleContours->total;
			CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
			while (cvGet2D(canvas, point->y, point->x).val[0] != 0 && safeCount--) {
				cvSeqPushFront(circleContours, point);
				cvSeqPop(circleContours, NULL);
				point = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
			}
		}

		NSLog2("segments");
		typedef struct ocvLine {
			CvPoint start;
			CvPoint end;
		} ocvLine;
		CvSeq* segmentsSeq = cvCreateSeq(CV_SEQ_ELTYPE_PTR , sizeof(CvSeq), sizeof(ocvLine), cvCreateMemStorage(0));

		// Find longest line segment
		ocvLine line;
		CvPoint *lastPoint = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
		char previousValue = (char)cvGet2D(canvas, lastPoint->y, lastPoint->x).val[0] != 0;
		int segmentIndex = 0;
		int counter = 0;
		int maxCounter = -1;
		int maxCounterIndex = -1;
		for (int i = 0; i < circleContours->total; i++) {
			CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, circleContours, i);
			char value = (char)cvGet2D(canvas, point->y, point->x).val[0] != 0;

			// Increase length on white pixels
			if (value) {
				counter++;
			}

			// Start counting on up-edge
			if (value != 0 && value != previousValue) {
				counter = 0;
				line.start = *point;
			}

			// Stop counting on down-edge and push line to sequence
			if (value == 0 && value != previousValue) {
				if (counter > maxCounter) {
					maxCounter = counter;
					maxCounterIndex = segmentIndex;
				}
				segmentIndex++;
				line.end = *point;
				cvSeqPush(segmentsSeq, &line);
			}

			previousValue = value;
		}
		cvReleaseMemStorage(&circleContours->storage);

		// Rotate line segments so longest is at first index
		while (maxCounterIndex--) {
			ocvLine *line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 0);
			cvSeqPush(segmentsSeq, line);
			cvSeqPopFront(segmentsSeq, NULL);
		}

		{ // Align fingerTipSeq to segmentsSeq
			NSLog2("align tip and segment");
			/*
			get wrist point
			foreach fingerTip
				save closest length and associated index
			shift fingertips index times
			*/
			ocvLine* line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 0);
			CvPoint wristPoint = cvPointMidPoint(line->start, line->end);
			CvPoint *firstPoint = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 0);
			int minDistance = cvPointDistance(wristPoint, *firstPoint);
			int minDistanceIndex = 0;
			for (int i = 0; i < fingerTipSeq->total; i++) {
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, i);
				int distance = cvPointDistance(wristPoint, *point);
				if (distance < minDistance) {
					minDistance = distance;
					minDistanceIndex = i;
				}
			}

			// Rotate line segments so longest is at first index
			while (minDistanceIndex--) {
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 0);
				cvSeqPush(fingerTipSeq, point);
				cvSeqPopFront(fingerTipSeq, NULL);
			}
		}

		{ // find thumb
			NSLog2("find thumb");
			ocvLine *lineN = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 1);
			ocvLine *lineP = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, -1);

			int distN = cvPointDistance(lineN->start, lineN->end);
			int distP = cvPointDistance(lineP->start, lineP->end);

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			myHand->orientation = 0;
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			if (distP > distN) {
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				myHand->orientation = 1;
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				// Revert segmentsSeq
				ocvLine *line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 0);
				cvSeqPush(segmentsSeq, line);
				cvSeqPopFront(segmentsSeq, NULL);
				cvSeqInvert(segmentsSeq);
				// Revert fingerTipSeq
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 0);
				cvSeqPush(fingerTipSeq, point);
				cvSeqPopFront(fingerTipSeq, NULL);
				cvSeqInvert(fingerTipSeq);
			}
		}

		if (01) { // Print finger + wrist count
			// Print thumb and index lines
			CvPoint *pointT = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 1);
			CvPoint *pointI = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 2);
			cvLine(overlay, center, *pointT, CV_RGB(22, 220, 22), 1, 8, 0);
			cvLine(overlay, center, *pointI, CV_RGB(220, 22, 22), 1, 8, 0);

			char buf[32];
			sprintf(buf, "%d", segmentIndex);
			drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), 1, center, CV_RGB(20, 20, 20));

			double angle = cvPointAngle(cvPointSubtract(*pointT, center), cvPointSubtract(*pointI, center));
			double phase = cvPointPhase(cvPointSubtract(*pointT, center));
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			myHand->controlAngle = angle;
			myHand->angle = phase + ((myHand->orientation) ? 1 : -1) * angle;
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			cvEllipse(overlay, center, cvSize(50, 50), 0, 180 / M_PI * (phase), 180 / M_PI * (myHand->angle), CV_RGB(200, 200, 200), 2, 8, 0);
			//cvCircle(overlay, cvPoint(0, 0), 100, CV_RGB(100, 20, 200), 2, 8, 0);
		}

		// Print finger segments
		#if 0
		NSLog2("print finger segments");
			for (int i = 0; i < segmentsSeq->total; i++) {
				ocvLine *line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, i);
				char buf[32];
				sprintf(buf, "%d", i);
				CvPoint lineCenter = cvPoint((line->end.x + line->start.x) / 2, (line->end.y + line->start.y) / 2);
				unsigned char angle = 127 + 127 * atan2(lineCenter.y - center.y, lineCenter.x - center.x) / M_PI;
				drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), .5, lineCenter, cvScalarRGBFromHSV(cvScalar(angle, 255, 127, 0)));
			}
		#endif
		cvReleaseMemStorage(&segmentsSeq->storage);

		// Print finger tips
		#if 0
		NSLog2("print finger tips");
			for (int i = 0; i < fingerTipSeq->total; i++) {
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, i);
				char buf[32];
				sprintf(buf, "%d", i);
				drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), .5, *point, CV_RGB(20, 20, 20));
			}
		#endif
	}

	NSLog2("cleanUp");
	// Clean up

	cvReleaseMemStorage(&fingerTipSeq->storage);
	cvReleaseMemStorage(&pointSeq->storage);

	cvReleaseImage(&canvas);

	return r;
}

CGImageRef operateImageRefCreate(CGImageRef imageRef0, CGImageRef imageRef1, NSMutableDictionary *options) {
	if (!imageRef0) { present(1, "!imageRef0"); return nil; }
	NSLog2("operating");

	IplImage *iplImage = IplImageFromCGImage(imageRef0);
	if (!iplImage) { present(1, "!iplImage"); return nil; }

	//test orientation handling
	//if ([options[@"inputType"] isEqualToString:@"image"]) { cvFlip(iplImage, NULL, 1); }

	if(0){ CvScalar s = cvScalarRGBFromHSV(CV_RGB(0,255,127)); s.val[3] = 255; } // for unused function warning

	IplImage *tmp3d = cvCreateImage(cvGetSize(iplImage), iplImage->depth, 3);
	cvCopy(iplImage, tmp3d,  NULL);

	IplImage *tmp1d = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 1);
	cvSet(tmp1d, cvScalarAll(255), NULL);

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

	//cvCvtColor(tmp3d, tmp3d, CV_BGR2RGB); goto end;

	if (0) { // Show masked image
		NSLog2("copy masked");
		cvCopy2(tmp3d, tmp3d, tmp1d);
	} else { // Show mask
		NSLog2("copy mask");
		cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
	}

	cvCvtColor(tmp3d, tmp3d, CV_BGR2RGB);
	//goto end;

	//cvCopy(iplImage, tmp3d, NULL);
	//cvSet(tmp3d, cvScalarAll(0), NULL);

	NSLog2("get contours");
	CvSeq *contourSeq = NULL;
	cvFindContours(tmp1d, cvCreateMemStorage(0), &contourSeq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	// Iterate over each contour
	//TRY_ONCE(
	NSLog2("foreach contour");
	for (CvSeq* seq = contourSeq; seq != 0; seq = seq->h_next) {
		// Ignore contours larger than half the screen
		//double area = cvContourArea(seq, CV_WHOLE_SEQ, 0);
		//if (area * 2 > tmp3d->width * tmp3d->height) continue;

		// Convex hull to contour area ratio
		//CvSeq *hull = cvConvexHull2(seq, NULL, CV_CLOCKWISE, 1);
		//double hullarea = cvContourArea(hull, CV_WHOLE_SEQ, 0);
		//double ratio = area / hullarea;
		//CvScalar color_fg = CV_RGB(sin(ratio + 0) * 127 + 128, sin(ratio + 2) * 127 + 128, (ratio + 4) * 127 + 128);

		IplImage *overlay = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 3);
		IplImage *textOverlay = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 3);

		// Extra canvas to do single contour drawing
		IplImage *canvas = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 1);
		cvDrawContours(canvas, seq, cvScalarAll(255), cvScalarAll(255), 0, CV_FILLED, 8, cvPoint(0, 0));
		//cvMerge(canvas, canvas, canvas, NULL, tmp3d); goto endLoop;

		// Defects
		// Assumed to be fingers (or wrist)
		NSLog2("get defects");
		CvSeq *defects = cvConvexityDefects(seq, cvConvexHull2(seq, NULL, CV_CLOCKWISE, 0), cvCreateMemStorage(0));
		if(01){ // Filter out the shallow ones
			NSLog2("deep defects");
			CvSeq* defectsSeq = cvCreateSeq(CV_SEQ_ELTYPE_PTR, sizeof(CvSeq), sizeof(CvConvexityDefect), cvCreateMemStorage(0));
			//cvDrawContours(overlay, defects, CV_RGB(0, 255, 0), cvScalarAll(0), 0, 1, 8, cvPoint(0, 0));
			for (int i = 0; i < defects->total; i++) {
				CvConvexityDefect *defect = CV_GET_SEQ_ELEM(CvConvexityDefect, defects, i);
				if (defect->depth > OCV_DEFECT_MIN_DEPTH) {
					cvSeqPush(defectsSeq, defect);
				}
			}
			cvReleaseMemStorage(&defects->storage);
			defects = defectsSeq;
			//cvClearMemStorage(defectsSeq->storage);
			//cvReleaseMemStorage(&defectsSeq->storage);
		}

		CvSeq* fingerTipSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), cvCreateMemStorage(0));
		CvSeq* pointSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), cvCreateMemStorage(0));

		NSLog2("get data");
		for (int i = 0; i < defects->total; i++) {
			CvConvexityDefect *defect = CV_GET_SEQ_ELEM(CvConvexityDefect, defects, i);
			cvSeqPush(pointSeq, defect->depth_point);

			CvConvexityDefect *prevDefect = CV_GET_SEQ_ELEM(CvConvexityDefect, defects, i - 1);
			CvPoint fingerTip = cvPointMidPoint(*defect->start, *prevDefect->end);
			//cvCircle(textOverlay, *defect->start, 2, CV_RGB(220, 20, 20), 2, 8, 0);
			//cvCircle(textOverlay, *prevDefect->end, 2, CV_RGB(20, 220, 20), 2, 8, 0);
			cvSeqPush(fingerTipSeq, &fingerTip);
		}
		//cvClearMemStorage(defects->storage);
		cvReleaseMemStorage(&defects->storage);
		//goto cleanUp;

		// Only convex contours are valid
		// Work with 3 to 6 deep defects
		if (cvCheckContourConvexity(pointSeq) && (pointSeq->total >= 3 && pointSeq->total <= 6)) {
			NSLog2("convex and 3-6 fingers");

			// Get circle enclosing defects
			// Not an excellent approximation of the palm, but close enough
			float radius = 0.0;
			CvPoint2D32f centerf;
			cvMinEnclosingCircle(pointSeq, &centerf, &radius);
			CvPoint center = cvPointFrom32f(centerf);
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			myHand.center = center;
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			{ // Place radius between shortest fingertip and longest deep defect
				NSLog2("radius setting");
				//find longest near radius
				float lnRadius = 0;
				for (int i = 0; i < pointSeq->total; i++) {
					CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, pointSeq, i);
					int distance = cvPointDistance(center, *point);
					if (distance > lnRadius) {
						lnRadius = distance;
					}
				}
				lnRadius += 2;

				//find shortest far radius
				float sfRadius = 1000;
				for (int i = 0; i < fingerTipSeq->total; i++) {
					CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, i);
					int distance = cvPointDistance(center, *point);
					if (distance < sfRadius) {
						sfRadius = distance;
					}
				}
				sfRadius -= 2;
				radius = (lnRadius + sfRadius) / 2;
				//cvCircle(overlay, center, lnRadius, CV_RGB(20, 220, 20), 2, 8, 0);
				//cvCircle(overlay, center, sfRadius, CV_RGB(20, 220, 20), 2, 8, 0);

				//cvCircle(overlay, center, radius, CV_RGB(20, 220, 20), 2, 8, 0);
			}
			//cvFillConvexPoly2(overlay, pointSeq, CV_RGB(30, 180, 20), 8, 0);

			// Find fingers
			CvSeq *circleContours = NULL;
			{ // Get contour of circle starting on black pixel
				NSLog2("circle");
				{ // Get circle points
					IplImage *tmp1d = cvCreateImage(cvGetSize(canvas), canvas->depth, 1);
					cvCircle(tmp1d, center, radius, cvScalarAll(255), CV_FILLED, 8, 0);
					cvFindContours(tmp1d, cvCreateMemStorage(0), &circleContours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
					cvReleaseImage(&tmp1d);
				}

				NSLog2("circle rotate");
				// Rotate sequence so that the first item contains a black pixel
				int safeCount = circleContours->total;
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
				while (cvGet2D(canvas, point->y, point->x).val[0] != 0 && safeCount--) {
					cvSeqPushFront(circleContours, point);
					cvSeqPop(circleContours, NULL);
					point = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
				}
			}

			NSLog2("segments");
			typedef struct ocvLine {
				CvPoint start;
				CvPoint end;
			} ocvLine;
			CvSeq* segmentsSeq = cvCreateSeq(CV_SEQ_ELTYPE_PTR , sizeof(CvSeq), sizeof(ocvLine), cvCreateMemStorage(0));

			// Find longest line segment
			ocvLine line;
			CvPoint *lastPoint = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
			char previousValue = (char)cvGet2D(canvas, lastPoint->y, lastPoint->x).val[0] != 0;
			int segmentIndex = 0;
			int counter = 0;
			int maxCounter = -1;
			int maxCounterIndex = -1;
			for (int i = 0; i < circleContours->total; i++) {
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, circleContours, i);
				char value = (char)cvGet2D(canvas, point->y, point->x).val[0] != 0;

				// Increase length on white pixels
				if (value) {
					counter++;
				}

				// Start counting on up-edge
				if (value != 0 && value != previousValue) {
					counter = 0;
					line.start = *point;
				}

				// Stop counting on down-edge and push line to sequence
				if (value == 0 && value != previousValue) {
					if (counter > maxCounter) {
						maxCounter = counter;
						maxCounterIndex = segmentIndex;
					}
					segmentIndex++;
					line.end = *point;
					cvSeqPush(segmentsSeq, &line);
				}

				previousValue = value;
			}
			//cvClearMemStorage(circleContours->storage);
			cvReleaseMemStorage(&circleContours->storage);

			// Rotate line segments so longest is at first index
			while (maxCounterIndex--) {
				ocvLine *line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 0);
				cvSeqPush(segmentsSeq, line);
				cvSeqPopFront(segmentsSeq, NULL);
			}

			{ // Align fingerTipSeq to segmentsSeq
				NSLog2("align tip and segment");
				/*
				get wrist point
				foreach fingerTip
					save closest length and associated index
				shift fingertips index times
				*/
				ocvLine* line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 0);
				CvPoint wristPoint = cvPointMidPoint(line->start, line->end);
				CvPoint *firstPoint = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 0);
				int minDistance = cvPointDistance(wristPoint, *firstPoint);
				int minDistanceIndex = 0;
				for (int i = 0; i < fingerTipSeq->total; i++) {
					CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, i);
					int distance = cvPointDistance(wristPoint, *point);
					if (distance < minDistance) {
						minDistance = distance;
						minDistanceIndex = i;
					}
				}

				// Rotate line segments so longest is at first index
				while (minDistanceIndex--) {
					CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 0);
					cvSeqPush(fingerTipSeq, point);
					cvSeqPopFront(fingerTipSeq, NULL);
				}
			}

			{ // find thumb
				NSLog2("find thumb");
				ocvLine *lineN = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 1);
				ocvLine *lineP = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, -1);

				int distN = cvPointDistance(lineN->start, lineN->end);
				int distP = cvPointDistance(lineP->start, lineP->end);

				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				myHand.orientation = 0;
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				if (distP > distN) {
					/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					myHand.orientation = 1;
					/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

					//cvLine(overlay, line->start, lineP->end, CV_RGB(22, 220, 22), 1, 8, 0);
					// Revert segmentsSeq
					ocvLine *line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 0);
					cvSeqPush(segmentsSeq, line);
					cvSeqPopFront(segmentsSeq, NULL);
					cvSeqInvert(segmentsSeq);
					// Revert fingerTipSeq
					CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 0);
					cvSeqPush(fingerTipSeq, point);
					cvSeqPopFront(fingerTipSeq, NULL);
					cvSeqInvert(fingerTipSeq);
				}
			}

			if (01) { // Print finger + wrist count
				// Print thumb and index lines
				CvPoint *pointT = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 1);
				CvPoint *pointI = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 2);
				cvLine(overlay, center, *pointT, CV_RGB(22, 220, 22), 1, 8, 0);
				cvLine(overlay, center, *pointI, CV_RGB(220, 22, 22), 1, 8, 0);

				char buf[32];
				sprintf(buf, "%d", segmentIndex);
				drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), 1, center, CV_RGB(20, 20, 20));

				double angle = cvPointAngle(cvPointSubtract(*pointT, center), cvPointSubtract(*pointI, center));
				double phase = cvPointPhase(cvPointSubtract(*pointT, center));
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				myHand.controlAngle = angle;
				myHand.angle = phase + ((myHand.orientation) ? 1 : -1) * angle;
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				#if 0
					sprintf(buf, "%.1f", 180 / M_PI * angle);
					drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), .5, cvPoint(textOverlay->width - 50, 50), cvScalar(200, 15, 127, 0));
					sprintf(buf, "%.1f", 180 / M_PI * phase);
					drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), .5, cvPoint(50, 50), cvScalar(200, 15, 255, 0));
					sprintf(buf, "%.1f", 180 / M_PI * (phase + ((myHand.orientation) ? 1 : -1) * angle));
					drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), .5, cvPoint(textOverlay->width/2, 50), cvScalar(200, 15, 255, 0));
				#endif

				cvEllipse(overlay, center, cvSize(50, 50), 0, 180 / M_PI * (phase), 180 / M_PI * (phase + ((myHand.orientation) ? 1 : -1) * angle), CV_RGB(200, 200, 200), 2, 8, 0);
				//cvCircle(overlay, cvPoint(0, 0), 100, CV_RGB(100, 20, 200), 2, 8, 0);
			}

			// Print finger segments
			#if 0
			NSLog2("print finger segments");
				for (int i = 0; i < segmentsSeq->total; i++) {
					ocvLine *line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, i);
					//cvLine(overlay, line->start, line->end, CV_RGB(22, 22, 22), 1, 8, 0);
					char buf[32];
					sprintf(buf, "%d", i);
					CvPoint lineCenter = cvPoint((line->end.x + line->start.x) / 2, (line->end.y + line->start.y) / 2);
					unsigned char angle = 127 + 127 * atan2(lineCenter.y - center.y, lineCenter.x - center.x) / M_PI;
					drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), .5, lineCenter, cvScalarRGBFromHSV(cvScalar(angle, 255, 127, 0)));
				}
			#endif
			//cvClearMemStorage(segmentsSeq->storage);
			cvReleaseMemStorage(&segmentsSeq->storage);

			// Print finger tips
			#if 0
			NSLog2("print finger tips");
				for (int i = 0; i < fingerTipSeq->total; i++) {
					CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, i);
					//cvCircle(textOverlay, *point, 10, CV_RGB(20, 20, 20), 2, 8, 0);
					char buf[32];
					sprintf(buf, "%d", i);
					drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), .5, *point, CV_RGB(20, 20, 20));
				}
			#endif
		}

		goto cleanUp;
		cleanUp:;
		NSLog2("cleanUp");
		// Clean up

		//cvClearMemStorage(fingerTipSeq->storage);
		cvReleaseMemStorage(&fingerTipSeq->storage);
		//cvClearMemStorage(pointSeq->storage);
		cvReleaseMemStorage(&pointSeq->storage);

		cvReleaseImage(&canvas);

		// Copy overlay and text to output image
		cvCopyNonZero(overlay, tmp3d, NULL);
		cvReleaseImage(&overlay);
		cvCopyNonZero(textOverlay, tmp3d, NULL);
		cvReleaseImage(&textOverlay);
	}
	//)

	//cvClearMemStorage(contourSeq->storage);
	cvReleaseMemStorage(&contourSeq->storage);

	goto end;
	end:;

	cvReleaseImage(&tmp1d);
	cvReleaseImage(&iplImage);

	CGImageRef imageRef = CGImageFromIplImage(tmp3d);
	cvReleaseImage(&tmp3d);
	#if 0
	static int reportCount = 0;
	if (!(reportCount %= (30 * 2))) {
		present(0, "size: %dx%d", CGImageGetWidth(imageRef), CGImageGetHeight(imageRef));
	} reportCount++;
	#endif
	return imageRef;
}


UIImage *operateImageCreate(UIImage *image0, UIImage *image1, NSMutableDictionary *options) {
	CGImageRef imageRef0 = image0.CGImage;
	CGImageRef imageRef1 = image1.CGImage;
	#if 0
	int scale = 0;
	float floatingValue = options[@"floatingValue"] ? ((NSNumber *)options[@"floatingValue"]).floatValue : 1;
	NSLog2(([NSString stringWithFormat:@"floatingValue %f", floatingValue]).UTF8String);
	// Scale down input image
	if (scale) { CGImageRef tmp = CreateScaledCGImageFromCGImage(imageRef0, floatingValue); if (tmp) { CGImageRelease(imageRef0); imageRef0 = tmp; } }
	#endif

	CGImageRef imageRefOut = operateImageRefCreate(imageRef0, imageRef1, options);

	#if 0
	// Scale up output image
	if (scale) { CGImageRef tmp = CreateScaledCGImageFromCGImage(imageRefOut, 1/floatingValue); if (tmp) { CGImageRelease(imageRefOut); imageRefOut = tmp; } }
	#endif

	UIImage *uiImage = [[UIImage alloc] initWithCGImage:imageRefOut];
	CGImageRelease(imageRefOut);

	return uiImage;
}
