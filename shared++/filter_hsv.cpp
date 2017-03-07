#include "filter_hsv.hpp"

void maskByHSVMat(cv::Mat& src, cv::Mat& dst, cv::Scalar minHSV, cv::Scalar maxHSV) {
	std::vector<cv::Mat> rgbChannels(3);

	{ // Get HSV values
		src.copyTo(dst);
		cv::GaussianBlur(dst, dst, cv::Size(9, 9), 0, 0, 0);
		cv::cvtColor(dst, dst, cv::COLOR_RGB2HSV);
		cv::split(dst, rgbChannels);
	}
//return;
	// Filter each range separately since cvInRange doesn't handle low limit > high limit (special case for hue range)
	if (minHSV[0] < maxHSV[0]) {
		cv::inRange(rgbChannels[0], cv::Scalar(minHSV[0]), cv::Scalar(maxHSV[0]), rgbChannels[0]);
	} else {
		cv::Mat tmp1d;
		cv::inRange(rgbChannels[0], cv::Scalar(0), cv::Scalar(maxHSV[0]), tmp1d);
		cv::inRange(rgbChannels[0], cv::Scalar(minHSV[0]), cv::Scalar(255), rgbChannels[0]);
		//cvOr(&tmp1d, &rgbChannels[0], &rgbChannels[0]);
		// cv::bitwise_or(tmp1d, rgbChannels[0], rgbChannels[0]);
		// cv::threshold(rgbChannels[0], rgbChannels[0], 0, 255, cv::THRESH_BINARY);
	}
	cv::inRange(rgbChannels[1], cv::Scalar(minHSV[1]), cv::Scalar(maxHSV[1]), rgbChannels[1]);
	cv::inRange(rgbChannels[2], cv::Scalar(minHSV[2]), cv::Scalar(maxHSV[2]), rgbChannels[2]);

	// Generate mask
	cv::merge(rgbChannels, dst);
	cv::cvtColor(dst, dst, cv::COLOR_HSV2RGB);
	//CVSHOW("skin2", dst->width*6/5, dst->height*2/3, dst->width/2, dst->height/2, dst);
	cv::threshold(dst, dst, 240, 255, cv::THRESH_BINARY);
}

void filterByHSVMat(cv::Mat& src, cv::Mat& dst, cv::Scalar minHSV, cv::Scalar maxHSV) {

}
