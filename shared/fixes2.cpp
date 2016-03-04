#include "fixes2.hpp"
#include <opencv2/core/core.hpp>

double getTickCount() {
	return cv::getTickCount();
}

double getTickFrequency() {
    return cv::getTickFrequency();
}
