#import <UIKit/UIKit.h>

@interface AppDelegate: NSObject <UIApplicationDelegate>

@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) UINavigationController *navViewController;
@property (nonatomic, retain) UIViewController *rootViewController;

- (void)refreshDefaultPNG;
- (UIImage *)makeDefaultImage;

@end
