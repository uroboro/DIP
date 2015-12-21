#import <UIKit/UIKit.h>

int main(int argc, char **argv) {
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	NSString *appClassName = [NSBundle mainBundle].infoDictionary[@"UIApplicationClass"];
	NSString *appDelegateClassName = [NSBundle mainBundle].infoDictionary[@"UIApplicationDelegateClass"];
	int r = UIApplicationMain(argc, argv, appClassName, appDelegateClassName);
	[pool release];
	return r;
}
