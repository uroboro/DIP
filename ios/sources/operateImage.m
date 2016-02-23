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

	if(s == 0) {
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

CGImageRef operateImageRefCreate(CGImageRef imageRef0, CGImageRef imageRef1, NSMutableDictionary *options) {
	if (!imageRef0) { present(1, "!imageRef0"); return nil; }
	//if (!imageRef1) { present(1, "!imageRef1"); return nil; }

	IplImage *iplImage = IplImageFromCGImage(imageRef0);

	IplImage *iplImageOut = NULL;

	#define DO_ONCE(block) { static dispatch_once_t once ## __LINE__; dispatch_once(&once ## __LINE__, block); }
	#define cvPointDistance(p, q) sqrt(((q).x - (p).x) * ((q).x - (p).x) + ((q).y - (p).y) * ((q).y - (p).y))
	#define cvPointMidPoint(p, q) cvPoint(((q).x + (p).x) / 2, ((q).y + (p).y) / 2)

	#define OCV_GRAYSCALE_DISTANCE 20
	#define OCV_DEFECT_MIN_DEPTH 10
	#define OCV_OBJECT_WIDTH_HEIGHT_RATIO 2
	IplImage *tmp3d = cvCreateImage(cvGetSize(iplImage), iplImage->depth, 3);
	//ocvSamplify(iplImage, tmp3d, 10);
	//goto endProc;
	cvCvtColor(iplImage, tmp3d, CV_RGB2BGR);

	IplImage *tmp1d = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 1);

	{ // Distance from grayscale and threshold
		IplImage *tmp3d2 = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 3);
		ocvDistance2Grayscale(tmp3d, tmp3d2);
		cvThreshold(tmp3d2, tmp3d2, OCV_GRAYSCALE_DISTANCE, 255, CV_THRESH_BINARY);
		cvCvtColor(tmp3d2, tmp1d, CV_BGR2GRAY);
		cvReleaseImage(&tmp3d2);
		filterByVolume(tmp1d, tmp1d, tmp1d->width * tmp1d->height / 100);
		cvSmooth(tmp1d, tmp1d, CV_GAUSSIAN, 9, 0, 0, 0);
		cvThreshold(tmp1d, tmp1d, 127, 255, CV_THRESH_BINARY);
		cvCopy2(tmp3d, tmp3d, tmp1d);
		//cvCvtColor(tmp3d, tmp3d, CV_BGR2RGB); goto endProc;
	}

	{ // Filter out non skin tones
		CvScalar minScalar = cvScalar(160, 30, 50, 0);
		CvScalar maxScalar = cvScalar(30, 255, 255, 255);
		maskByHSV(tmp3d, minScalar, maxScalar, tmp1d);

		filterByVolume(tmp1d, tmp1d, tmp1d->width * tmp1d->height / 100);
		cvSmooth(tmp1d, tmp1d, CV_GAUSSIAN, 9, 0, 0, 0);
		cvThreshold(tmp1d, tmp1d, 127, 255, CV_THRESH_BINARY);
	}

	{ // Analise each contour
		CvMemStorage *contourStorage = cvCreateMemStorage(0);
		CvSeq *contourSeq = NULL;
		cvFindContours(tmp1d, contourStorage, &contourSeq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
		for (CvSeq* seq = contourSeq; seq != 0; seq = seq->h_next) {
			// Ignore contours with a ratio of w:h or h:w greater than OCV_OBJECT_WIDTH_HEIGHT_RATIO
			CvBox2D box = cvMinAreaRect2(seq, NULL);
			double ratio = box.size.width / box.size.height;
			if (ratio > OCV_OBJECT_WIDTH_HEIGHT_RATIO || ratio < 1 / OCV_OBJECT_WIDTH_HEIGHT_RATIO) {
				//cvBox2(tmp3d, box, cvScalarAll(255), CV_FILLED, 8, 0);
				cvDrawContours(tmp1d, seq, cvScalarAll(0), cvScalarAll(0), 0, CV_FILLED, 8, cvPoint(0, 0));
			} else {
				cvDrawContours(tmp1d, seq, cvScalarAll(255), cvScalarAll(255), 0, CV_FILLED, 8, cvPoint(0, 0));
			}
		}
		cvClearMemStorage(contourStorage);
	}

	if (01) { // Show masked image
		cvCopy2(tmp3d, tmp3d, tmp1d);
	} else { // Show mask
		cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
	}

	cvCvtColor(tmp3d, tmp3d, CV_BGR2RGB);
	//goto endProc;

	//cvCopy(iplImage, tmp3d, NULL);
	//cvSet(tmp3d, cvScalarAll(0), NULL);

	CvSeq *contourSeq = NULL;
	cvFindContours(tmp1d, cvCreateMemStorage(0), &contourSeq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	//TRYONCE((^{ // Iterate over each contour
	for (CvSeq* seq = contourSeq; seq != 0; seq = seq->h_next) {
		// Ignore contours larger than half the screen
		//double area = cvContourArea(seq, CV_WHOLE_SEQ, 0);
		//if (area * 2 > tmp3d->width * tmp3d->height) continue;

		// Convex hull to contour area ratio
		//CvSeq *hull = cvConvexHull2(seq, NULL, CV_CLOCKWISE, 1);
		//double hullarea = cvContourArea(hull, CV_WHOLE_SEQ, 0);
		//double ratio = area / hullarea;
		//CvScalar color_fg = CV_RGB(sin(ratio + 0) * 127 + 128, sin(ratio + 2) * 127 + 128, (ratio + 4) * 127 + 128);

		// Extra canvas to do single contour drawing
		IplImage *canvas = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 1);
		cvDrawContours(canvas, seq, cvScalarAll(255), cvScalarAll(255), 0, CV_FILLED, 8, cvPoint(0, 0));
		//cvMerge(canvas, canvas, canvas, NULL, tmp3d); goto endLoop;

		IplImage *overlay = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 3);
		IplImage *textOverlay = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 3);

		// Defects
		// Assumed to be fingers (or wrist)
		CvSeq *defects = cvConvexityDefects(seq, cvConvexHull2(seq, NULL, CV_CLOCKWISE, 0), NULL);
		{ // Filter out the shallow ones
			CvSeq* defectsSeq = cvCreateSeq(CV_SEQ_ELTYPE_PTR, sizeof(CvSeq), sizeof(CvConvexityDefect), cvCreateMemStorage(0));
			//cvDrawContours(overlay, defects, CV_RGB(0, 255, 0), cvScalarAll(0), 0, 1, 8, cvPoint(0, 0));
			for (int i = 0; i < defects->total; i++) {
				CvConvexityDefect *defect = CV_GET_SEQ_ELEM(CvConvexityDefect, defects, i);
				if (defect->depth > OCV_DEFECT_MIN_DEPTH) {
					cvSeqPush(defectsSeq, defect);
				}
			}
			// Trick to not use memory storage
			CvSeq *storageFreeSeq = cvConvexityDefects(seq, cvConvexHull2(seq, NULL, CV_CLOCKWISE, 0), NULL);
			cvSeqPopMulti(storageFreeSeq, NULL, storageFreeSeq->total, 0);
			for (int i = 0; i < defectsSeq->total; i++) {
				cvSeqPush(storageFreeSeq, CV_GET_SEQ_ELEM(CvConvexityDefect, defectsSeq, i));
			}
			defects = storageFreeSeq;
			cvClearMemStorage(defectsSeq->storage);
		}

		if (0) DO_ONCE(^{
		present(0, ([NSString stringWithFormat:@"%d\n", (int)defects->total].UTF8String));
		});

		#define DO_FINGERTIPS 01
		#if DO_FINGERTIPS
		CvSeq* fingerTipSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), cvCreateMemStorage(0));
		#endif

		CvSeq* pointSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), cvCreateMemStorage(0));

		//find longest near radius
		//find shortest far radius
		for (int i = 0; i < defects->total; i++) {
			CvConvexityDefect *defect = CV_GET_SEQ_ELEM(CvConvexityDefect, defects, i);
			cvSeqPush(pointSeq, defect->depth_point);

			#if DO_FINGERTIPS
			CvConvexityDefect *prevDefect = CV_GET_SEQ_ELEM(CvConvexityDefect, defects, i - 1);
			CvPoint fingerTip = cvPointMidPoint(*defect->start, *prevDefect->end);
			//cvCircle(textOverlay, *defect->start, 2, CV_RGB(220, 20, 20), 2, 8, 0);
			//cvCircle(textOverlay, *prevDefect->end, 2, CV_RGB(20, 220, 20), 2, 8, 0);
			cvSeqPush(fingerTipSeq, &fingerTip);
			#endif
		}
		//goto cleanUp;

		// Only convex contours are valid
		// Work with 3 to 6 deep defects
		if (cvCheckContourConvexity(pointSeq) && (pointSeq->total >= 3 && pointSeq->total <= 6)) {
			// Get circle enclosing defects
			// Not an excellent approximation of the palm, but close enough
			float radius = 0.0;
			CvPoint2D32f centerf;
			cvMinEnclosingCircle(pointSeq, &centerf, &radius);
			CvPoint center = cvPointFrom32f(centerf);

			//cvDrawContours(overlay, hull, color_fg, cvScalarAll(0), 0, 1, 8, cvPoint(0, 0));
			//cvFillConvexPoly2(overlay, pointSeq, CV_RGB(30, 180, 20), 8, 0);

			// make this radius always fit bewteen minimum defect depth and minumum finger length
			#define OCV_FINGER_RADIUS 1.3 * radius
			// Find fingers
			CvSeq *circleContours = NULL;
			{ // Get contour of circle starting on black pixel
				{ // Get circle points
					IplImage *tmp1d = cvCreateImage(cvGetSize(canvas), canvas->depth, 1);
					cvCircle(tmp1d, center, OCV_FINGER_RADIUS, cvScalarAll(255), CV_FILLED, 8, 0);
					cvFindContours(tmp1d, cvCreateMemStorage(0), &circleContours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
					cvReleaseImage(&tmp1d);
				}

				// Rotate sequence so that the first item contains a black pixel
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
				while (cvGet2D(canvas, point->y, point->x).val[0] != 0) {
					cvSeqPushFront(circleContours, point);
					cvSeqPop(circleContours, NULL);
					point = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
				}
			}

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
			cvClearMemStorage(circleContours->storage);

			// Rotate line segments so longest is at first index
			while (maxCounterIndex--) {
				ocvLine *line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 0);
				cvSeqPush(segmentsSeq, line);
				cvSeqPopFront(segmentsSeq, NULL);
			}

			{ // Print finger + wrist count
				char buf[32];
				sprintf(buf, "%d", segmentIndex);
				drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), 1, center, CV_RGB(20, 20, 20));
			}

			if (01) { // Align fingerTipSeq to segmentsSeq
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
				//present(0, ([NSString stringWithFormat:@"dist to %d: %d\n", minDistanceIndex, minDistance].UTF8String));

				// Rotate line segments so longest is at first index
				while (minDistanceIndex--) {
					CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 0);
					cvSeqPush(fingerTipSeq, point);
					cvSeqPopFront(fingerTipSeq, NULL);
				}
			}

			if (01) { // find thumb
				ocvLine *lineN = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 1);
				ocvLine *lineP = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, -1);

				int distN = cvPointDistance(lineN->start, lineN->end);
				int distP = cvPointDistance(lineP->start, lineP->end);
				if (distP > distN) {
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
				} else {
					//cvLine(overlay, line->end, lineN->start, CV_RGB(22, 220, 22), 1, 8, 0);
				}
			}

			char buf[32];
			CvFont font;
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, .5, .5, 0, 1, 8);
			// Print finger segments
			if (01) for (int i = 0; i < segmentsSeq->total; i++) {
				ocvLine *line = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, i);
				cvLine(overlay, line->start, line->end, CV_RGB(22, 22, 22), 1, 8, 0);
				sprintf(buf, "%d", i);
				CvPoint lineCenter = cvPoint((line->end.x + line->start.x) / 2, (line->end.y + line->start.y) / 2);
				unsigned char angle = 127 + 127 * atan2(lineCenter.y - center.y, lineCenter.x - center.x) / M_PI;
				drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), .5, lineCenter, cvScalarRGBFromHSV(cvScalar(angle, 255, 127, 0)));
			}
			cvClearMemStorage(segmentsSeq->storage);

			// Print finger tips
			#if DO_FINGERTIPS
			for (int i = 0; i < fingerTipSeq->total; i++) {
				CvPoint *point = CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, i);
				//cvCircle(textOverlay, *point, 10, CV_RGB(20, 20, 20), 2, 8, 0);
				sprintf(buf, "%d", i);
				CvPoint lineCenter = *point;
				drawBadge(textOverlay, buf, CV_RGB(200, 200, 200), .5, lineCenter, CV_RGB(20, 20, 20));
			}
			#endif

		}

		goto cleanUp;
		cleanUp:;
		// Clean up
		//cvClearMemStorage(defectStorage);
		#if DO_FINGERTIPS
		cvClearMemStorage(fingerTipSeq->storage);
		#endif
		cvClearMemStorage(pointSeq->storage);

		cvReleaseImage(&canvas);

		// Copy overlay and text to output image
		cvCopyNonZero(overlay, tmp3d, NULL);
		cvReleaseImage(&overlay);
		cvCopyNonZero(textOverlay, tmp3d, NULL);
		cvReleaseImage(&textOverlay);
	}
	//}));

	cvClearMemStorage(contourSeq->storage);

	goto endProc;
	endProc:;
	cvReleaseImage(&tmp1d);

	//cvCvtColor(tmp3d, tmp3d, CV_BGR2RGB);
	iplImageOut = tmp3d;

	cvReleaseImage(&iplImage);

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
