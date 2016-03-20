#include "messages.h"
#include "UIImage+OpenCV.h"

#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgcodecs/imgcodecs_c.h>
#include <opencv2/imgcodecs/ios.h>

IplImage *IplImageFromCGImage(CGImageRef imageRef) {
	IplImage *iplImage = NULL;
	if (imageRef) {
		size_t width = CGImageGetWidth(imageRef);
		size_t height = CGImageGetHeight(imageRef);
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

		IplImage *iplImage4 = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);

		CGContextRef context = CGBitmapContextCreate(iplImage4->imageData,
			iplImage4->width, iplImage4->height, iplImage4->depth, iplImage4->widthStep,
			colorSpace, kCGImageAlphaPremultipliedLast|kCGBitmapByteOrderDefault);
		CGColorSpaceRelease(colorSpace);

		CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
		CGContextRelease(context);

		iplImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
		cvCvtColor(iplImage4, iplImage, CV_RGBA2RGB);
		cvReleaseImage(&iplImage4);
	}
	return iplImage;
}

IplImage *IplImageFromUIImage(UIImage *image) {
	return IplImageFromCGImage(image.CGImage);
}

void CVMatFromCGImage(CGImageRef imageRef, cv::Mat& imageMat) {
	if (imageRef) {
		size_t width = CGImageGetWidth(imageRef);
		size_t height = CGImageGetHeight(imageRef);
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

		imageMat.create(height, width, CV_8UC4);

		CGContextRef context = CGBitmapContextCreate(imageMat.data,
			imageMat.cols, imageMat.rows, 8, imageMat.step[0],
			colorSpace, kCGImageAlphaPremultipliedLast|kCGBitmapByteOrderDefault);
		CGColorSpaceRelease(colorSpace);

		CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
		CGContextRelease(context);

		cv::Mat imageMat3 = cv::Mat(height, width, CV_8UC3);
		cv::cvtColor(imageMat, imageMat3, cv::COLOR_RGBA2RGB);
		imageMat3.copyTo(imageMat);
	}
}

static CGImageRef newImageRefWithData(unsigned char *data, unsigned int width, unsigned int height) {
	CGImageRef imageRef = NULL;
	if (data) {
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		int bitsPerComponent = 8;
		int bytesPerRow = 4 * width;
		CGContextRef context = CGBitmapContextCreate(data,
			width, height, bitsPerComponent, bytesPerRow,
			colorSpace, kCGImageAlphaPremultipliedLast|kCGBitmapByteOrderDefault);
		CGColorSpaceRelease(colorSpace);

		imageRef = CGBitmapContextCreateImage(context);
		CGContextRelease(context);
	}
	return imageRef;
}

CGImageRef CGImageFromIplImage(IplImage *image) {
	CGImageRef imageRef = nil;
	IplImage *tmp3d = NULL;
	if (image) {
		if (image->nChannels == 1) {
			tmp3d = cvCreateImage(cvGetSize(image), image->depth, 3);
			cvMerge(image, image, image, NULL, tmp3d);
			IplImage *swap = image;
			image = tmp3d;
			tmp3d = swap;
		}
		unsigned int width = image->width;
		unsigned int height = image->height;

		unsigned char *data = (unsigned char *)calloc(width * height * 4, sizeof(unsigned char *));
		for (unsigned int y = 0; y < height; y++) {
			for (unsigned int x = 0; x < width; x++) {
				CvScalar px = cvGet2D(image, y, x);
				data[4 * (y * width + x) + 0] = (int)px.val[0];
				data[4 * (y * width + x) + 1] = (int)px.val[1];
				data[4 * (y * width + x) + 2] = (int)px.val[2];
				data[4 * (y * width + x) + 3] = 255;
			}
		}
		imageRef = newImageRefWithData(data, width, height);
		free(data);
	}
	if (tmp3d) {
		IplImage *swap = image;
		image = tmp3d;
		tmp3d = swap;
		cvReleaseImage(&tmp3d);
	}
	return imageRef;
}

UIImage *UIImageFromIplImage(IplImage *image) {
	UIImage *uiImage = nil;
	if (image) {
		CGImageRef imageRef = CGImageFromIplImage(image);
		uiImage = [UIImage imageWithCGImage:imageRef];
		CGImageRelease(imageRef);
	}
	return uiImage;
}

CGImageRef CGImageFromCVMat(cv::Mat& image) {
	CGImageRef imageRef = nil;
	if (!image.empty()) {
		if (image.channels() == 1) {
			cv::Mat tmp3d;
			cv::Mat matArray[3] = { image, image, image };
			cv::merge(matArray, 3, tmp3d);
			tmp3d.copyTo(image);
		}
		unsigned int width = image.cols;
		unsigned int height = image.rows;

		unsigned char *data = (unsigned char *)calloc(width * height * 4, sizeof(unsigned char *));
		for (unsigned int y = 0; y < height; y++) {
			for (unsigned int x = 0; x < width; x++) {
				cv::Vec3b px = image.at<cv::Vec3b>(cv::Point(x, y));
				data[4 * (y * width + x) + 0] = (int)px.val[0];
				data[4 * (y * width + x) + 1] = (int)px.val[1];
				data[4 * (y * width + x) + 2] = (int)px.val[2];
				data[4 * (y * width + x) + 3] = 255;
			}
		}
		imageRef = newImageRefWithData(data, width, height);
		free(data);
	}
	return imageRef;
}

@implementation UIImage (IplImage)

+ (UIImage *)imageWithIplImage:(IplImage *)image {
	return UIImageFromIplImage(image);
}

- (IplImage *)iplImage {
	return IplImageFromUIImage(self);
}

+ (UIImage *)imageWithCVMat:(cv::Mat&)image {
	return MatToUIImage(image);
}

- (cv::Mat)CVMat {
	cv::Mat mat;
	UIImageToMat(self, mat, 1);
	return mat;
}

@end
