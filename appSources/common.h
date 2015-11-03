#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
#define DIP_EXTERN extern "C"
#define DIP_EXTERN_BEGIN extern "C" {
#define DIP_EXTERN_END }
#else
#define DIP_EXTERN
#define DIP_EXTERN_BEGIN
#define DIP_EXTERN_END
#endif

#ifdef __OBJC__
#include <objc/runtime.h>
#import <UIKit/UIAlertView.h>

#define UIAlert(t, m) dispatch_async(dispatch_get_main_queue(), ^{ [[[[objc_getClass("UIAlertView") alloc] initWithTitle:(t) message:(m) delegate:0 cancelButtonTitle:@"OK" otherButtonTitles:0] autorelease] show]; })
#endif

#endif /* COMMON_H */
