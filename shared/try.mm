#include <opencv2/core/core.hpp>
#include "try.h"

int tryCPP(void (^block)(void)) {
    int b = 1;
    try {
        block();
    }
    catch (cv::Exception& e) {
        const char* err_msg = e.what();
        [UIPasteboard generalPasteboard].string = @(err_msg);
        UIAlert(@"OpenCV exception caught", [@"The following error has been copied to the clipboard:\n" stringByAppendingString:@(err_msg)]);
        b = 0;
    }
    return b;
}
