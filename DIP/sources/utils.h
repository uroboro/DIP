#ifndef UTILS_H
#define UTILS_H

#include <Foundation/Foundation.h>
#include <CoreGraphics/CGGeometry.h>

NSString *UtilsDocumentPathWithName(NSString *name);
NSString *UtilsResourcePathWithName(NSString *name);

CGRect UtilsAvailableScreenRect();

#endif