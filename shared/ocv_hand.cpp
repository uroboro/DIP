#include "ocv_hand.h"
#include <stdio.h>

#include "filter_grayscale.h"
#include "filter_hsv.h"
#include "filter_volume.h"

#include "geometry.h"
#include "drawing.h"
#include "fixes.h"

void ocvCreateHandIconWithHand(IplImage *layer, IplImage *sprite, ocvHand myHand) {
	CvSize spriteSize = cvGetSize(sprite);

	if (myHand.orientation) {
		cvFlip(sprite, NULL, 1);
	}

	{
		IplImage *tmp1d = cvCreateImage(cvGetSize(sprite), sprite->depth, 1);
		cvCvtColor(sprite, tmp1d, CV_RGB2GRAY);
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


#define OCV_DEFECT_MIN_DEPTH 10
#define OCV_OBJECT_WIDTH_HEIGHT_RATIO 2
#define OCV_ACCUMULATOR_ALPHA 0.3

void ocvPrefilterImageMask(IplImage *src, IplImage *dst, int grayscaleDistance, CvScalar minScalar, CvScalar maxScalar) {
	IplImage *tmp1dg = cvCreateImage(cvGetSize(dst), dst->depth, 1);
	IplImage *tmp1dc = cvCreateImage(cvGetSize(dst), dst->depth, 1);

	{ // Distance from grayscale and threshold
		NSLog2("grayscale");
		maskByDistance2Grayscale(src, tmp1dg, grayscaleDistance);
		CVSHOW("grayscale", tmp1dg->width/2, 0, tmp1dg->width/2, tmp1dg->height/2, tmp1dg);
	}

	{ // Filter out non skin tones
		NSLog2("skin");
		maskByHSV(src, tmp1dc, minScalar, maxScalar);
		CVSHOW("skin", tmp1dc->width/2, 0, tmp1dc->width/2, tmp1dc->height/2, tmp1dc);
	}

	{
		IplImage *white = cvCloneImage(tmp1dc); cvSet(white, cvScalarAll(255), NULL);
		IplImage *tmp3d = cvCreateImage(cvGetSize(dst), dst->depth, 3);
		cvMerge(tmp1dg, white, tmp1dc, NULL, tmp3d);
		cvCvtColor(tmp3d, white, CV_RGB2GRAY);
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
				// Ignore contours larger than half the screen
				//double area = cvContourArea(seq, CV_WHOLE_SEQ, 0);
				//if (area * 2 > tmp3d->width * tmp3d->height) continue;

				// Convex hull to contour area ratio
				//CvSeq *hull = cvConvexHull2(seq, NULL, CV_CLOCKWISE, 1);
				//double hullarea = cvContourArea(hull, CV_WHOLE_SEQ, 0);
				//double ratio = area / hullarea;
				//CvScalar color_fg = CV_RGB(sin(ratio + 0) * 127 + 128, sin(ratio + 2) * 127 + 128, (ratio + 4) * 127 + 128);

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

int ocvAnalizeContour(CvSeq *seq, IplImage *overlay, ocvHand *myHand) {
	int r = 0;

	// Canvas for contour analizing
	IplImage *canvas = cvCreateImage(cvGetSize(overlay), overlay->depth, 1);
	cvDrawContours(canvas, seq, cvScalarAll(255), cvScalarAll(255), 0, CV_FILLED, 8, cvPoint(0, 0));

	CvSeq* fingerTipSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), cvCreateMemStorage(0));
	CvSeq* pointSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), cvCreateMemStorage(0));

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
		NSLog2("convex and 3-6 fingers");

		// Get circle enclosing defects
		// Not an excellent approximation of the palm, but close enough
		float radius = 0.0;
		CvPoint2D32f centerf;
		cvMinEnclosingCircle(pointSeq, &centerf, &radius);
		CvPoint center = cvPointFrom32f(centerf);

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

		NSLog2("segments");
		typedef struct ocvLine {
			CvPoint start;
			CvPoint end;
		} ocvLine;
		CvSeq* segmentsSeq = cvCreateSeq(CV_SEQ_ELTYPE_PTR , sizeof(CvSeq), sizeof(ocvLine), cvCreateMemStorage(0));

		int fingerCount = 0;

		{ // Find fingers
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
				while (cvGet2D(canvas, point->y, point->x).val[0] != 0 && safeCount-- > 0) {
					char buf[32]; sprintf(buf, "circle rotate loop: %d", safeCount); NSLog2(buf);
					cvSeqPushFront(circleContours, point);
					cvSeqPop(circleContours, NULL);
					point = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
				}
				if (safeCount < 0) {
					cvReleaseMemStorage(&circleContours->storage);
					goto cleanUp;
				}
			}

			int maxCounterIndex = -1;
			// Find longest line segment
			ocvLine line;
			CvPoint *lastPoint = CV_GET_SEQ_ELEM(CvPoint, circleContours, circleContours->total - 1);
			char previousValue = (char)cvGet2D(canvas, lastPoint->y, lastPoint->x).val[0] != 0;
			int counter = 0;
			int maxCounter = -1;
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
						maxCounterIndex = fingerCount;
					}
					fingerCount++;
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

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		myHand->orientation = 1;
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		{ // find thumb
			NSLog2("find thumb");
			ocvLine *lineN = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, 1);
			ocvLine *lineP = CV_GET_SEQ_ELEM(ocvLine, segmentsSeq, -1);

			int distN = cvPointDistance(lineN->start, lineN->end);
			int distP = cvPointDistance(lineP->start, lineP->end);

			if (distP > distN) {
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				myHand->orientation = 0;
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

		{ // Save hand info
			r = 1;
			myHand->fingers = fingerCount - 1;
			myHand->center = center;
			myHand->thumbTip = *CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 1);
			myHand->indexTip = *CV_GET_SEQ_ELEM(CvPoint, fingerTipSeq, 2);
			myHand->controlAngle = cvPointAngle(cvPointSubtract(myHand->thumbTip, myHand->center), cvPointSubtract(myHand->indexTip, myHand->center));
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
				drawBadge(overlay, buf, CV_RGB(200, 200, 200), .5, lineCenter, cvScalarRGBFromHSV(cvScalar(angle, 255, 127, 0)));
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
				drawBadge(overlay, buf, CV_RGB(200, 200, 200), .5, *point, CV_RGB(20, 20, 20));
			}
		#endif
	}

	goto cleanUp;
	cleanUp:;
	NSLog2("cleanUp");
	// Clean up

	cvReleaseMemStorage(&fingerTipSeq->storage);
	cvReleaseMemStorage(&pointSeq->storage);

	cvReleaseImage(&canvas);

	return r;
}

void ocvDrawHandInfo(IplImage *overlay, ocvHand myHand) {
	double phaseT = cvPointPhase(cvPointSubtract(myHand.thumbTip, myHand.center));
	double phaseI = cvPointPhase(cvPointSubtract(myHand.indexTip, myHand.center));
	// Draw thumb and index lines
	cvLine(overlay, myHand.center, myHand.thumbTip, CV_RGB(22, 220, 22), 2, 8, 0);
	cvLine(overlay, myHand.center, myHand.indexTip, CV_RGB(220, 22, 22), 2, 8, 0);
	cvCircle(overlay, myHand.thumbTip, 5, CV_RGB(20, 20, 20), CV_FILLED, 8, 0);
	cvCircle(overlay, myHand.thumbTip, 7, CV_RGB(255, 255, 255), 2, 8, 0);
	cvCircle(overlay, myHand.indexTip, 5, CV_RGB(20, 20, 20), CV_FILLED, 8, 0);
	cvCircle(overlay, myHand.indexTip, 7, CV_RGB(255, 255, 255), 2, 8, 0);
	// Draw arc
	cvEllipse(overlay, myHand.center, cvSize(50, 50), 0, 180 / M_PI * (phaseT), 180 / M_PI * (phaseI), CV_RGB(22, 22, 220), 2, 8, 0);
	// Draw finger count
	char buf[32]; sprintf(buf, "%d", myHand.fingers);
	drawBadge(overlay, buf, CV_RGB(200, 200, 200), 1, myHand.center, CV_RGB(20, 20, 20));
}

#include "try.h"
#include "fixes2.hpp"

#define OCV_GRAYSCALE_DISTANCE 20

DIP_EXTERN IplImage *ocv_handSpriteCreate(char *path);

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
			cvCvtColor(tmp3d, tmp1d, CV_RGB2GRAY);
			cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
		}

		//goto end;
	#endif

	CvScalar minScalar = cvScalar(160, 30, 50, 0);
	CvScalar maxScalar = cvScalar(30, 255, 255, 255);
	TRY_ONCE(
	ocvPrefilterImageMask(tmp3d, tmp1d, OCV_GRAYSCALE_DISTANCE, minScalar, maxScalar);
	)

	if (01) { // Show masked image
		if (0) { // Show masked image
			NSLog2("copy masked");
			cvCopy2(tmp3d, tmp3d, tmp1d);
		} else { // Show mask
			NSLog2("copy mask");
			cvMerge(tmp1d, tmp1d, tmp1d, NULL, tmp3d);
		}
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
	{ char buf[32]; sprintf(buf, "foreach contour (%d)", contourCount); NSLog2(buf); }
#if 01
	if (contourCount > 0) {
		for (CvSeq* seq = contourSeq; seq != 0; seq = seq->h_next) {
			IplImage *overlay = cvCreateImage(cvGetSize(tmp3d), tmp3d->depth, 3);

			__block ocvHand myHand;

			NSLog2("analize contour");
			__block int ret = 0;
			TRY_ONCE(
			ret = ocvAnalizeContour(seq, overlay, &myHand);
			)

	if (ret) {
		CvFont font; double fontSize = 1; cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, fontSize, fontSize, 0, 2, 8);
		char buf[32]; sprintf(buf, "maybe hand");
		cvPutText(tmp3d, buf, cvPoint(10, 30), &font, CV_RGB(0, 255, 0));
	} else {
		CvFont font; double fontSize = 1; cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, fontSize, fontSize, 0, 2, 8);
		char buf[32]; sprintf(buf, "no hand");
		cvPutText(tmp3d, buf, cvPoint(10, 30), &font, CV_RGB(0, 255, 0));
	}

			// Copy overlay and text to output image
			if (ret && (myHand.fingers > 2 && myHand.fingers < 6)) {
				NSLog2("draw hand");
				#if 0
				ocvDrawHandInfo(overlay, myHand);
				#else
				char *handPaths[] = { "1456448219_icon_2_rock_n_roll.png", "1456448230_icon_3_high_five.png" };
				IplImage *sprite = ocv_handSpriteCreate(handPaths[(myHand.fingers == 5)]);

				ocvCreateHandIconWithHand(overlay, sprite, myHand);
				cvReleaseImage(&sprite);
				#endif
				cvCopyNonZero(overlay, tmp3d, NULL);
			} else {
				cvDrawContours(red3d, seq, CV_RGB(0,0,255), CV_RGB(0,0,255), 0, CV_FILLED, 8, cvPoint(0, 0));
			}
			cvReleaseImage(&overlay);
		}
		cvReleaseMemStorage(&contourSeq->storage);
	}
#endif
	goto end;
	end:;
	//cvAddWeighted(tmp3d, 0.7, red3d, 0.3, 0, tmp3d);

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
