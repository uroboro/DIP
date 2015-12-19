#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>
#include <objc/message.h>
#include <CoreFoundation/CFNotificationCenter.h>

#define UIAlert(t, m) dispatch_async(dispatch_get_main_queue(), ^{ [[[[objc_getClass("UIAlertView") alloc] initWithTitle:(t) message:(m) delegate:0 cancelButtonTitle:(id)CFSTR("OK") otherButtonTitles:0] autorelease] show]; })

typedef enum DBGOutputMode {
	DBGOutputModeGUI,
	DBGOutputModeTTY,
	DBGOutputModeSyslog
} DBGOutputMode;

static DBGOutputMode outputMode = DBGOutputModeTTY;

int present(char mode, const char format[], ...) {
	int r = 0;
	if (!format) { present(1, "!format"); return r; }

	va_list args;
	va_start(args, format);
//	const char *syslogPrefix = "\x1b[1;34m[IRCyslog]\x1b[0m";
	NSString *message = [[NSString alloc] initWithFormat:@(format) arguments:args];
	va_end(args);

	switch (outputMode) {
	case DBGOutputModeGUI:
		UIAlert((mode ? @"ERROR" : @"WARNING"), message);
		break;
	case DBGOutputModeTTY:
		r = fprintf(mode ? stderr : stdout, "%s\n", message.UTF8String);
		break;
	case DBGOutputModeSyslog:
		syslog(mode ? LOG_ERR : LOG_WARNING, "%s", message.UTF8String);
		break;
	}
	[message release];

	return r;
}

static void callback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
	outputMode = DBGOutputModeGUI; 
}

__attribute__((constructor)) static void ctor(int argc, char **argv, char **envp) {
	outputMode = ([UIApplication sharedApplication]) ? DBGOutputModeGUI : (isatty(STDIN_FILENO)) ? DBGOutputModeTTY : DBGOutputModeSyslog;

	CFNotificationCenterAddObserver(
		CFNotificationCenterGetLocalCenter(),
		NULL, callback,
		(CFStringRef)UIApplicationDidFinishLaunchingNotification,
		NULL, CFNotificationSuspensionBehaviorCoalesce);

	return;
}
