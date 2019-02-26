#include "filter_grayscale.hpp"

static void ocvDistance2GrayscaleFunc(cv::Vec3b& p, const int *pos) {
	p = cv::Vec3b::all(sqrt(2.0 / 3 * (
		  p[0] * (p[0] - p[1])
		+ p[1] * (p[1] - p[2])
		+ p[2] * (p[2] - p[0])
	)));
}

void ocvDistance2GrayscaleMat(cv::Mat& src, cv::Mat& dst) {
	src.copyTo(dst);
	if (src.channels() == 3) {
		#if CV_MAJOR_VERSION >= 3
			dst.forEach<cv::Vec3b>(&ocvDistance2GrayscaleFunc);
		#else
			for (unsigned int y = 0; y < src.size().height; y++) {
				for (unsigned int x = 0; x < src.size().width; x++) {
					ocvDistance2GrayscaleFunc(dst.at<cv::Vec3b>(cv::Point(x, y)), NULL);
				}
			}
		#endif
	}
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
