#ifndef OPERATEIMAGE_H
#define OPERATEIMAGE_H

#include "common.h"

#import <UIKit/UIImage.h>

DIP_EXTERN NSUInteger startingOperation(void);
DIP_EXTERN NSUInteger maxOperations(void);

DIP_EXTERN CGImageRef operateImageRefCreate(CGImageRef imageRef0, CGImageRef imageRef1, NSMutableDictionary *options);
DIP_EXTERN UIImage *operateImageCreate(UIImage *image0, UIImage *image1, NSMutableDictionary *options);

#endif /* OPERATEIMAGE_H */
