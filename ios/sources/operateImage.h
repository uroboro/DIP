#ifndef OPERATEIMAGE_H
#define OPERATEIMAGE_H

#include "common.h"

#import <UIKit/UIImage.h>

DIP_EXTERN_BEGIN

CGImageRef operateImageRefCreate(CGImageRef imageRef, NSMutableDictionary *options);

DIP_EXTERN_END

#endif /* OPERATEIMAGE_H */
