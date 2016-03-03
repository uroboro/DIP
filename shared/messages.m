#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>

#if __OBJC__
#include <objc/message.h>
#include <CoreFoundation/CFNotificationCenter.h>
#endif

#include "messages.h"

typedef enum DBGOutputMode {
	DBGOutputModeTTY    = 1,
	DBGOutputModeSyslog = 2,
#if __OBJC__
	DBGOutputModeGUI    = 3
#endif
} DBGOutputMode;

static DBGOutputMode outputMode = DBGOutputModeTTY;

int present(char mode, const char format[], ...) {
	int r = 0;
	if (!format) { present(1, "!format"); return r; }

	char message[1024] = { 0 };

	va_list args;
	va_start(args, format);
	vsprintf(message, format, args);
	va_end(args);

	switch (outputMode) {
	case DBGOutputModeTTY:
		r = fprintf(mode ? stderr : stdout, "%s\n", message);
		break;
	case DBGOutputModeSyslog:
		syslog(mode ? LOG_ERR : LOG_WARNING, "%s", message);
		break;
#if __OBJC__
	case DBGOutputModeGUI: {
			NSString *m = [NSString stringWithUTF8String:message];
			UIAlert((mode ? @"ERROR" : @"WARNING"), m);
		}
		break;
#endif
	}

	return r;
}

#if __OBJC__
static void callback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
	outputMode = DBGOutputModeGUI; 
}
#endif

__attribute__((constructor)) static void ctor(int argc, char **argv, char **envp) {
	outputMode = (isatty(STDIN_FILENO)) ? DBGOutputModeTTY : DBGOutputModeSyslog;

#if __OBJC__
	outputMode = ([UIApplication sharedApplication]) ? DBGOutputModeGUI : outputMode;

	CFNotificationCenterAddObserver(
		CFNotificationCenterGetLocalCenter(),
		NULL, callback,
		(CFStringRef)UIApplicationDidFinishLaunchingNotification,
		NULL, CFNotificationSuspensionBehaviorCoalesce);
#endif
}
