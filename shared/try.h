#ifndef TRYCPP_H
#define TRYCPP_H

#include "common.h"

#if defined(__OBJC__)
typedef void (^tryBlock)(void);
#else
typedef void *tryBlock;
#endif

DIP_EXTERN int tryCPP(tryBlock);

#endif /* TRYCPP_H */
