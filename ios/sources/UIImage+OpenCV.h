#ifndef UIIMAGE_OPENCV_H
#define UIIMAGE_OPENCV_H

#include <CoreGraphics/CGImage.h>

#include "common.h"

DIP_EXTERN_BEGIN

IplImage *IplImageFromCGImage(CGImageRef imageRef);
CGImageRef CGImageFromIplImage(IplImage *image);

#ifdef __cplusplus
void CVMatFromCGImage(CGImageRef imageRef, cv::Mat& imageMat);
CGImageRef CGImageFromCVMat(cv::Mat& image);
#endif

DIP_EXTERN_END

#ifdef __OBJC__
#import <UIKit/UIImage.h>

DIP_EXTERN_BEGIN

IplImage *IplImageFromUIImage(UIImage *image);
UIImage *UIImageFromIplImage(IplImage *image);

DIP_EXTERN_END

@interface UIImage (IplImage)
+ (UIImage *)imageWithIplImage:(IplImage *)image;
- (IplImage *)iplImage;
#ifdef __cplusplus
+ (UIImage *)imageWithCVMat:(cv::Mat&)image;
- (cv::Mat)CVMat;
#endif
@end
#endif

#endif /* UIIMAGE_OPENCV_H */
