#ifndef OPERATEIMAGE_H
#define OPERATEIMAGE_H

#include "common.h"

#import <UIKit/UIImage.h>

DIP_EXTERN_BEGIN

IplImage *ocv_handSpriteCreate(char *path);

CGImageRef operateImageRefCreate(CGImageRef imageRef);

void operateImageProcessImageAndUpdateView(CGImageRef imageRef, UIImageView *imageView, NSDictionary *options);

DIP_EXTERN_END

#endif /* OPERATEIMAGE_H */
