#import <Availability.h>
#import <UIKit/UIGraphics.h>
#import <QuartzCore/CALayer.h>

#include "common.h"
#include "utils.h"
#import "AppDelegate.h"

UIKIT_EXTERN NSString *rvcName(void);

@interface UIDevice (PrivateCategoryToAvoidWarnings)
- (void)setOrientation:(UIInterfaceOrientation)orientation;
@end

@implementation AppDelegate

#pragma mark - Monitoring Application State Changes

- (BOOL)_application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	Class rootVCClass = NSClassFromString(rvcName());
	if (!rootVCClass) {
		UIAlert(@"Error", ([NSString stringWithFormat:@"No \n%@\n class in code", rvcName()]));
		return NO;
	}

	_rootViewController = [[rootVCClass alloc] init];

	if (!_rootViewController) {
		UIAlert(@"Error", ([NSString stringWithFormat:@"Unable to init %@\n class", rvcName()]));
		return NO;
	}

	_navViewController = [[UINavigationController alloc] initWithRootViewController:_rootViewController];

	_window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[_window setAutoresizesSubviews:YES];
	[_window setAutoresizingMask:(UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight)];
	[_window setRootViewController:_navViewController];

	return YES;
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
// Available in iOS 2.0 and later.
	[self _application:application didFinishLaunchingWithOptions:nil];
	[_window makeKeyAndVisible];
}

- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
// Available in iOS 6.0 and later.
	BOOL r = [self _application:application didFinishLaunchingWithOptions:nil];
	return r;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
// Available in iOS 3.0 and later.
	BOOL r = YES;
	if (kCFCoreFoundationVersionNumber <= 800) { // Up to iOS 6
		r = [self _application:application didFinishLaunchingWithOptions:nil];
	}
	[_window makeKeyAndVisible];
	return YES;
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
// Available in iOS 2.0 and later.
}

- (void)applicationWillResignActive:(UIApplication *)application {
// Available in iOS 2.0 and later.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
// Available in iOS 4.0 and later.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
// Available in iOS 4.0 and later.
}

- (void)applicationWillTerminate:(UIApplication *)application {
// Available in iOS 2.0 and later.
	//Private API
	[[UIDevice currentDevice] setOrientation:UIInterfaceOrientationPortrait];
	[_navViewController popToRootViewControllerAnimated:NO];
	[self refreshDefaultPNG];
}

#pragma mark - Opening a URL Resource

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url {
// Available in iOS 2.0 and later.
	return YES;
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation {
// Available in iOS 4.2 and later.
	return YES;
}

#pragma mark - Managing Status Bar Changes

- (void)application:(UIApplication *)application willChangeStatusBarOrientation:(UIInterfaceOrientation)newStatusBarOrientation duration:(NSTimeInterval)duration {
// Available in iOS 2.0 and later.
}

- (void)application:(UIApplication *)application didChangeStatusBarOrientation:(UIInterfaceOrientation)oldStatusBarOrientation {
// Available in iOS 2.0 and later.
}

- (void)application:(UIApplication *)application willChangeStatusBarFrame:(CGRect)newStatusBarFrame {
// Available in iOS 2.0 and later.
}

- (void)application:(UIApplication *)application didChangeStatusBarFrame:(CGRect)oldStatusBarFrame {
// Available in iOS 2.0 and later.
}

#pragma mark - Responding to System Notifications

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
// Available in iOS 2.0 and later.
}

- (void)applicationSignificantTimeChange:(UIApplication *)application {
// Available in iOS 2.0 and later.
}

#pragma mark - Handling Remote Notifications

- (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo {
// Available in iOS 3.0 and later.
}

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken {
// Available in iOS 3.0 and later.
}

- (void)application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error {
// Available in iOS 3.0 and later.
}

#pragma mark - Handling Local Notifications

- (void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification {
// Available in iOS 4.0 and later.
}

#pragma mark - Responding to Content Protection Changes

- (void)applicationProtectedDataWillBecomeUnavailable:(UIApplication *)application {
// Available in iOS 4.0 and later.
}

- (void)applicationProtectedDataDidBecomeAvailable:(UIApplication *)application {
// Available in iOS 4.0 and later.
}

#pragma mark - Managing the Default Interface Orientations

- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window {
// Available in iOS 6.0 and later.
	return UIInterfaceOrientationMaskAllButUpsideDown;
}

#pragma mark -

- (void)dealloc {
	[_rootViewController release];
	_rootViewController = nil;
	[_navViewController release];
	_navViewController = nil;
	[_window release];
	_window = nil;
	[super dealloc];
}

#pragma mark - Custom methods

- (void)refreshDefaultPNG {
	UIImage *defaultImage = [self makeDefaultImage];
	if (defaultImage) {
		[UIImagePNGRepresentation(defaultImage) writeToFile:UtilsDocumentPathWithName(@"Default.png") atomically:YES];
	}
}

- (UIImage *)makeDefaultImage {
	UIViewController *vc = _rootViewController;
	UIGraphicsBeginImageContext(vc.view.bounds.size);
	[vc.view.layer renderInContext:UIGraphicsGetCurrentContext()];
	UIImage *viewImage = UIGraphicsGetImageFromCurrentImageContext();
	UIGraphicsEndImageContext();
	return viewImage;
}

@end
