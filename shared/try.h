#ifndef TRYCPP_H
#define TRYCPP_H

#include "common.h"

#define TRYONCE(block) static int tryAgain ## __LINE__ = 1; if (tryAgain ## __LINE__) { tryAgain ## __LINE__ = tryCPP(block); }

DIP_EXTERN int tryCPP(void (^)(void));

#endif /* TRYCPP_H */
