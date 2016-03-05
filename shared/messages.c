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

static DBGOutputMode _outputMode = DBGOutputModeTTY;

int present(char mode, const char format[], ...) {
	int r = 0;
	if (!format) { present(1, "!format"); return r; }

	char message[1024] = { 0 };

	va_list args;
	va_start(args, format);
	vsprintf(message, format, args);
	va_end(args);

	char outputLevel = mode & 0x01;
	char outputMode = mode & 0xFE;
	if (!outputMode) {
		outputMode = _outputMode;
	}
	switch (outputMode) {
	case DBGOutputModeTTY:
		r = fprintf(outputLevel ? stderr : stdout, "%s\n", message);
		break;
	case DBGOutputModeSyslog:
		syslog(outputLevel ? LOG_ERR : LOG_WARNING, "%s", message);
		break;
#if __OBJC__
	case DBGOutputModeGUI: {
			NSString *m = [NSString stringWithUTF8String:message];
			UIAlert((outputLevel ? @"ERROR" : @"WARNING"), m);
		}
		break;
#endif
	}

	return r;
}

#if __OBJC__
static void callback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
	_outputMode = DBGOutputModeGUI;
}
#endif

__attribute__((constructor)) static void ctor(int argc, char **argv, char **envp) {
	_outputMode = (isatty(STDIN_FILENO)) ? DBGOutputModeTTY : DBGOutputModeSyslog;

#if __OBJC__
	_outputMode = ([UIApplication sharedApplication]) ? DBGOutputModeGUI : outputMode;

	CFNotificationCenterAddObserver(
		CFNotificationCenterGetLocalCenter(),
		NULL, callback,
		(CFStringRef)UIApplicationDidFinishLaunchingNotification,
		NULL, CFNotificationSuspensionBehaviorCoalesce);
#endif
}
