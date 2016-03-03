#ifndef IMAGE_H
#define IMAGE_H

#import <UIKit/UIImage.h>
#import <AVFoundation/AVFoundation.h>

#import "common.h"

DIP_EXTERN_BEGIN

// http://stackoverflow.com/a/20611860/1429562
UIImage *UIImageCreateApplyingMetadata(UIImage *image);

CGImageRef CGImageCreateMirrorImage(CGImageRef imageRef);

CGImageRef CGImageCreateScaled(CGImageRef image, CGFloat scale);

CGImageRef CGImageCreateFromSampleBuffer(CMSampleBufferRef sampleBuffer);

DIP_EXTERN_END

#endif
