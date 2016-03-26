#ifndef DIP_OPENCV
#define DIP_OPENCV

#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>

#ifdef DIP_DESKTOP
#include <opencv2/highgui/highgui_c.h>
#endif

#ifdef __cplusplus
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#if CV_VERSION_MAJOR >= 3
#include <opencv2/imgcodecs.hpp>
#endif

#endif

#endif /* DIP_OPENCV */
