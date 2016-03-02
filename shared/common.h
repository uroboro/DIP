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

#if __OBJC__
#include <objc/runtime.h>
#import <UIKit/UIAlertView.h>

#define UIAlert(t, m) dispatch_async(dispatch_get_main_queue(), ^{ [[[[objc_getClass("UIAlertView") alloc] initWithTitle:[(id)(t) description] message:[(id)(m) description] delegate:0 cancelButtonTitle:@"OK" otherButtonTitles:0] autorelease] show]; })

#else
#define UIAlert(t, m)
#define NSLog(...)
#endif

#define NSLog2(message) NSLog(@"XXX Reached line \e[31m%d\e[0m, message: \e[32m%s\e[0m", __LINE__, message);

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define __windows__ 1
#endif


#if __linux__ || __windows__
#define CVSHOW(name, x, y, w, h, image) { cvNamedWindow(name, 0); if (x > 0 && y > 0) cvMoveWindow(name, x, y); cvResizeWindow(name, w, h); cvShowImage(name, image); }
#else
#define CVSHOW(name, x, y, w, h, image)
#endif

#if __windows__
#define RESOURCES "C:\\Users\\uroboro\\Documents\\GitHub\\DIP\\DIP\\Resources\\"
#else
#define RESOURCES "Resources/"
#endif

#endif /* COMMON_H */
