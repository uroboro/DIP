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

#define _STR(x)					#x
#define STR(x)					_STR(x)
#define _CAT(x,y)				x##y
#define CAT(x,y)				_CAT(x,y)

#if DIP_DARWIN
	#include <dispatch/dispatch.h>
	#define DO_ONCE(block) { static dispatch_once_t CAT(once,__LINE__); dispatch_once(& CAT(once,__LINE__), ^{block}); }
	#define TRY_ONCE(block) { static int CAT(tryAgain,__LINE__) = 1; if (CAT(tryAgain,__LINE__)) { CAT(tryAgain,__LINE__) = tryCPP(^{block}); }; CAT(tryAgain,__LINE__); }
#else
	#define __block
	//#warning Using non-clang "blocks"
	#define DO_ONCE(block) { static int CAT(once,__LINE__) = 1; if (CAT(once,__LINE__)) { CAT(once,__LINE__) = 0; ({block}); } }

	#ifdef __cplusplus
		#define TRY_ONCE(block) { static int CAT(tryAgain,__LINE__) = 1; if (CAT(tryAgain,__LINE__)) { CAT(tryAgain,__LINE__) =\
				({ int b = 1; try { block; }\
				catch (cv::Exception& e) {\
					const char* err_msg = e.what(); char buf[1024]; sprintf(buf, "OpenCV exception caught: %s", err_msg); present(0, buf); b = 0;\
				};\
				b; });\
			}; CAT(tryAgain,__LINE__); }
	#else
		#define TRY_ONCE(block) block
	#endif
#endif

#include "messages.h"
#if DIP_MOBILE && __OBJC__
	#import <objc/runtime.h>
	#import <objc/message.h>
	#include <dispatch/dispatch.h>

	#define UIAlert(t, m) dispatch_async(dispatch_get_main_queue(), ^{ \
		id a = objc_msgSend(objc_getClass("UIAlertView"), sel_registerName("alloc"));\
		objc_msgSend(a, sel_registerName("initWithTitle:message:delegate:cancelButtonTitle:otherButtonTitles:"), objc_msgSend((t), sel_registerName("description")), objc_msgSend((m), sel_registerName("description")), nil, CFSTR("OK"), nil);\
		objc_msgSend(a, sel_registerName("show"));\
		objc_msgSend(a, sel_registerName("release"));\
	})
	#define NSLog2(format, ...) NSLog(@"XXX Reached \e[33m%s\e[m \e[31m%d\e[m, message: \e[32m" format "\e[m", __FILE__, __LINE__, ##__VA_ARGS__);
#else
	#define UIAlert(t, m)
	#define NSLog(...)
	#define NSLog2(format, ...) present(0, "XXX Reached \e[33m%s\e[m \e[31m%d\e[m, message: \e[32m" format "\e[m", __FILE__, __LINE__, ##__VA_ARGS__);
#endif

#if DIP_DESKTOP
#define _CVSHOW(name, x, y, w, h, image) { cvNamedWindow(name, 0); cvMoveWindow(name, x, y); cvResizeWindow(name, w, h); cvShowImage(name, image); }
#define CVSHOW(name, x, y, w, h, image) {\
	if (image->nChannels == 1) {\
		IplImage * CAT(tmp3d,__LINE__) = cvCreateImage(cvGetSize(image), image->depth, 3);\
		cvMerge(image, image, image, NULL, CAT(tmp3d,__LINE__));\
		_CVSHOW(name, x, y, w, h, CAT(tmp3d,__LINE__));\
		cvReleaseImage(& CAT(tmp3d,__LINE__));\
	} else { _CVSHOW(name, x, y, w, h, image); }\
}
#else
#define CVSHOW(name, x, y, w, h, image)
#endif

#if DIP_WINDOWS
#define RESOURCES "C:\\Users\\uroboro\\Documents\\GitHub\\DIP\\DIP\\Resources\\"
#else
#define RESOURCES "Resources/"
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include "opencv.h"
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include "opencv.h"
#pragma GCC diagnostic pop
#endif

#endif /* COMMON_H */
