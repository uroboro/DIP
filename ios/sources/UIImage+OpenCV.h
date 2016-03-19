#ifndef UIIMAGE_OPENCV_H
#define UIIMAGE_OPENCV_H

#include <CoreGraphics/CGImage.h>

#include "common.h"

DIP_EXTERN IplImage *IplImageFromCGImage(CGImageRef imageRef);
DIP_EXTERN CGImageRef CGImageFromIplImage(IplImage *image);

#ifdef __OBJC__
#import <UIKit/UIImage.h>

DIP_EXTERN IplImage *IplImageFromUIImage(UIImage *image);
DIP_EXTERN UIImage *UIImageFromIplImage(IplImage *image);

@interface UIImage (IplImage)
+ (UIImage *)imageWithIplImage:(IplImage *)image;
- (IplImage *)iplImage;
@end
#endif

#endif /* UIIMAGE_OPENCV_H */
