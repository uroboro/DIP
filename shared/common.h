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

#if defined(__APPLE__) && defined(__MACH__)
	/* Apple OSX and iOS (Darwin). ------------------------------ */
	#define DIP_DARWIN 1

	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR == 1
		/* iOS in Xcode simulator */

	#elif TARGET_OS_IPHONE == 1
		/* iOS on iPhone, iPad, etc. */
		#define DIP_MOBILE 1
	#elif TARGET_OS_MAC == 1
		/* OSX */
		#define DIP_DESKTOP 1
	#endif
#endif

#if defined(__linux__)
	/* Linux. --------------------------------------------------- */
	#define DIP_DESKTOP 1
#endif

#if defined(__CYGWIN__) && !defined(_WIN32)
	/* Cygwin POSIX under Microsoft Windows. -------------------- */
	#define DIP_WINDOWS 1
	#define DIP_DESKTOP 1
#elif defined(_WIN64)
	/* Microsoft Windows (64-bit). ------------------------------ */
	#define DIP_WINDOWS 1
	#define DIP_DESKTOP 1
#elif defined(_WIN32)
	/* Microsoft Windows (32-bit). ------------------------------ */
	#define DIP_WINDOWS 1
	#define DIP_DESKTOP 1
#endif

#if DIP_DARWIN
	#define DO_ONCE(block) { static dispatch_once_t once ## __LINE__; dispatch_once(&once ## __LINE__, ^{block}); }
#else
	#warning Using non-clang "blocks"
	#define DO_ONCE(block) { static int once ## __LINE__ = 1; if (once ## __LINE__) { once ## __LINE__ = 0; ({block}); } }
#endif

#if DIP_MOBILE && __OBJC__
	#include <objc/runtime.h>
	#import <UIKit/UIAlertView.h>

	#define UIAlert(t, m) dispatch_async(dispatch_get_main_queue(), ^{ [[[[objc_getClass("UIAlertView") alloc] initWithTitle:[(id)(t) description] message:[(id)(m) description] delegate:0 cancelButtonTitle:@"OK" otherButtonTitles:0] autorelease] show]; })
#else
	#define UIAlert(t, m)
	#define NSLog(...)
#endif

#define NSLog2(message) NSLog(@"XXX Reached line \e[31m%d\e[0m, message: \e[32m%s\e[0m", __LINE__, message);

#if DIP_DESKTOP
#define CVSHOW(name, x, y, w, h, image) { cvNamedWindow(name, 0); if (x > 0 && y > 0) cvMoveWindow(name, x, y); cvResizeWindow(name, w, h); cvShowImage(name, image); }
#else
#define CVSHOW(name, x, y, w, h, image)
#endif

#if DIP_WINDOWS
#define RESOURCES "C:\\Users\\uroboro\\Documents\\GitHub\\DIP\\DIP\\Resources\\"
#else
#define RESOURCES "Resources/"
#endif

#endif /* COMMON_H */
