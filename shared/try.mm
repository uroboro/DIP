#include "common.h"
#include "try.h"
#import <objc/message.h>

int tryCPP(void (^block)(void)) {
    int b = 1;
    try {
        block();
    }
    catch (cv::Exception& e) {
        const char* err_msg = e.what();
        objc_msgSend(objc_msgSend(objc_getClass("UIPasteboard"), sel_registerName("generalPasteboard")), sel_registerName("setString"), @(err_msg));
        UIAlert(@"OpenCV exception caught", [@"The following error has been copied to the clipboard:\n" stringByAppendingString:@(err_msg)]);
        b = 0;
    }
    return b;
}
