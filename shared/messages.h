#ifndef MESSAGES_H
#define MESSAGES_H

#include "common.h"

typedef enum DBGOutputMode {
	DBGOutputModeWarning = 0x00,
	DBGOutputModeError   = 0x01,
	DBGOutputModeTTY     = 0x02,
	DBGOutputModeSyslog  = 0x04,
#if __OBJC__
	DBGOutputModeGUI     = 0x08
#endif
} DBGOutputMode;

DIP_EXTERN int present(char mode, const char format[], ...);
// mode ? error : warning

#endif /* MESSAGES_H */
