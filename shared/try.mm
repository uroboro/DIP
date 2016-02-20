#include <opencv2/core/core.hpp>
#include "try.h"

int tryCPP(void (^block)(void)) {
    int b = 1;
    try {
        block();
    }
    catch (cv::Exception& e) {
        const char* err_msg = e.what();
        UIAlert(@"OpenCV exception caught", ([NSString stringWithFormat:@"%s", err_msg]));
        b = 0;
    }
    return b;
}
