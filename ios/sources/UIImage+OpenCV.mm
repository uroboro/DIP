#include "messages.h"
#include "UIImage+OpenCV.h"

#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgcodecs/imgcodecs_c.h>

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
	if (image) {
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

@implementation UIImage (IplImage)

+ (UIImage *)imageWithIplImage:(IplImage *)image {
	return UIImageFromIplImage(image);
}

- (IplImage *)iplImage {
	return IplImageFromUIImage(self);
}

@end
