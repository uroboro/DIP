#ifndef IMAGE_H
#define IMAGE_H

#import <UIKit/UIImage.h>

#import "common.h"

DIP_EXTERN_BEGIN

// http://stackoverflow.com/a/20611860/1429562
UIImage *scaleAndRotateImage(UIImage *image);

CGImageRef CreateScaledCGImageFromCGImage(CGImageRef image, CGFloat scale) {

DIP_EXTERN_END

#endif
