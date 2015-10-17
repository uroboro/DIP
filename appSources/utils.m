#include <UIKit/UIKit.h>

#import "utils.h"

NSString *UtilsDocumentPathWithName(NSString *name) {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectoryPath = [paths objectAtIndex:0];
	NSString *docPath = [documentsDirectoryPath stringByAppendingPathComponent:name];
	return docPath;
}

NSString *UtilsResourcePathWithName(NSString *name) {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectoryPath = [paths objectAtIndex:0];
	NSString *bundleName = [NSBundle mainBundle].infoDictionary[@"CFBundleName"];
	NSString *pathComponent = [NSString stringWithFormat:@"%@.app/%@", bundleName, name];
	NSString *resPath = [[documentsDirectoryPath stringByDeletingLastPathComponent] stringByAppendingPathComponent:pathComponent];
	return resPath;
}

CGRect UtilsAvailableScreenRect() {
	CGRect appRect = [[UIScreen mainScreen] applicationFrame];
	appRect = CGRectOffset(appRect, 0, -appRect.origin.y);
	UINavigationController *navC = (UINavigationController *)[[UIApplication sharedApplication].delegate performSelector:@selector(navViewController)];
	CGFloat navBarHeight = navC.navigationBar.frame.size.height;
	return CGRectOffset(CGRectIntersection(appRect, CGRectOffset(appRect, 0, navBarHeight)), 0, -navBarHeight);
}