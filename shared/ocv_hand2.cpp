#include "ocv_hand.h"
#include <stdio.h>

#include "filter_grayscale.h"
#include "filter_hsv.h"
#include "filter_volume.h"

#include "geometry.h"
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#include "drawing.h"
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"
#include "drawing.h"
#pragma GCC diagnostic pop
#endif
#include "fixes.h"
#include "fixes2.hpp"

#include "filter_grayscale.hpp"
#include "filter_hsv.hpp"
// dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0), ^{ char buf[32]; sprintf(buf, "src size %03dx%03d", src.size().width, src.size().height); NSLog2(buf); });

#define OCV_GRAYSCALE_DISTANCE 20

void ocv_handAnalysisMat(cv::Mat& src, cv::Mat& dst) {
	double _time = cv::getTickCount();
	//cv::cvtColor(src, dst, cv::COLOR_RGB2GRAY);
	src.copyTo(dst);
	//dst.create(src.size().height, src.size().width, CV_8UC3);
	cv::Mat grayscaleMask;
	maskByDistance2GrayscaleMat(src, grayscaleMask, OCV_GRAYSCALE_DISTANCE);
	cv::Mat hsvMask;
	cv::Scalar minHSV = cv::Scalar(160, 30, 50, 0);
	cv::Scalar maxHSV = cv::Scalar(30, 255, 255, 255);
	maskByHSVMat(src, hsvMask, minHSV, maxHSV);
	//cv::fastNlMeansDenoisingColored(src, dst); // 3s per image, too slow

	if ((01)) {
		cv::Mat tmp3d;
		cv::Mat black(src.size(), src.type(), cv::Scalar::all(0));
		cv::Mat matArray[3] = { grayscaleMask, hsvMask, black };
		cv::merge(matArray, 3, tmp3d);
		tmp3d.copyTo(dst);
	}

	if (dst.channels() == 1) {
		cv::Mat tmp3d;
		cv::Mat matArray[3] = { dst, dst, dst };
		cv::merge(matArray, 3, tmp3d);
		tmp3d.copyTo(dst);
	}
	{ // On-screen debug data
		_time = (cv::getTickCount() - _time) / cv::getTickFrequency();
		{
			char buf[32]; sprintf(buf, "time: %dms (%.1f fps)?", (int)(1000 * _time), 1/_time);
			try {
				cv::putText(dst, buf, cv::Point(10, dst.rows - 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0, 255), 2, 8);
			}
			catch( cv::Exception& e ) {
			    const char* err_msg = e.what();
				// char astring[555];
				// sprintf(astring, "exception caught: %s", err_msg);
				// NSLog2(astring);
				NSLog2("exception caught: %s", err_msg);
			}
		}
		{
			char buf[32]; sprintf(buf, "size: %dx%d", dst.cols, dst.rows);
			cv::putText(dst, buf, cv::Point(10, dst.rows - 10), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0, 255), 2, 8);
		}
	}
}
