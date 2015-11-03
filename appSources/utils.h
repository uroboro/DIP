#ifndef UTILS_H
#define UTILS_H

#include <Foundation/Foundation.h>
#include <CoreGraphics/CGGeometry.h>

#include "common.h"

DIP_EXTERN NSString *UtilsDocumentPathWithName(NSString *name);
DIP_EXTERN NSString *UtilsResourcePathWithName(NSString *name);

DIP_EXTERN CGRect UtilsAvailableScreenRect();

#endif