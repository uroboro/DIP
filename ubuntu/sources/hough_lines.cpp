#include "hough_lines.h"
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/imgproc.hpp>

void houghLines(IplImage *src, IplImage *dst) {
	IplImage *tmp1d = NULL;
	if (src->nChannels == 1) {
		tmp1d = cvCloneImage(src);
	} else {
		tmp1d = cvCreateImage(cvGetSize(src), src->depth, 1);
		cvCvtColor(src, tmp1d, CV_RGB2GRAY);
	}
	cvCanny(tmp1d, tmp1d, 50, 200, 3);
	std::vector<cv::Vec4i> lines;
	HoughLinesP(cv::Mat(tmp1d), lines, 1, CV_PI/180, 50, 50, 10);
	for (size_t i = 0; i < lines.size(); i++) {
		cv::Vec4i l = lines[i];
		cvLine(dst, cvPoint(l[0], l[1]), cvPoint(l[2], l[3]), CV_RGB(0,0,255), 3, CV_AA);
	}
	cvReleaseImage(&tmp1d);
}
