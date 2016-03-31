#include "filter_grayscale.hpp"

//#include "try.h"
//#include "fixes2.hpp"

#if DIP_MOBILE
//#include <dispatch/dispatch.h>
#else
//#define dispatch_async(...)
#endif

static void ocvDistance2GrayscaleFunc(cv::Vec3b& p, const int *pos) {
	p = cv::Vec3b::all(sqrt(2.0 / 3 * (
		  p[0] * (p[0] - p[1])
		+ p[1] * (p[1] - p[2])
		+ p[2] * (p[2] - p[0])
	)));
}

void ocvDistance2GrayscaleMat(cv::Mat& src, cv::Mat& dst) {
	#if CV_VERSION_MAJOR >= 3
		src.copyTo(dst);
		dst.forEach<cv::Vec3b>(&ocvDistance2GrayscaleFunc);
	#else
		if (src.channels() == 3) {
			for (unsigned int y = 0; y < src.size().height; y++) {
				for (unsigned int x = 0; x < src.size().width; x++) {
					cv::Vec3b p = src.at<cv::Vec3b>(cv::Point(x, y));
					// Calculate distance to grayscale value
					dst.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b::all(sqrt(2.0 / 3 * (
						  p[0] * (p[0] - p[1])
						+ p[1] * (p[1] - p[2])
						+ p[2] * (p[2] - p[0])
					)));
				}
			}
		}
	#endif
}

void maskByDistance2GrayscaleMat(cv::Mat& src, cv::Mat& dst, int minDistance) {
	ocvDistance2GrayscaleMat(src, dst);
	cv::GaussianBlur(dst, dst, cv::Size(9, 9), 0, 0, 0);
	cv::cvtColor(dst, dst, cv::COLOR_RGB2GRAY);
	#if DIP_DESKTOP
	cv::equalizeHist(dst, dst);
	#endif
	//CVSHOW("grayscale2", dst->width*2/3, dst->height*2/3, dst->width/2, dst->height/2, dst);
	#if DIP_MOBILE
	cv::threshold(dst, dst, minDistance, 255, cv::THRESH_BINARY);
	#endif
}
