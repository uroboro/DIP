#import <UIKit/UIView.h>
#import <UIKit/UIImage.h>

#import "AVCustomCapture.h"

@interface OCVImageOperator : NSObject <AVCustomCaptureDelegate> {
}
@property (nonatomic, assign) CALayer *customPreviewLayer;
@property (nonatomic, assign) UIView *view;
- (id)initWithView:(UIView *)view;

@property (nonatomic, retain) UIImage *image;
- (UIImage *)operateImage:(UIImage *)image;

@property (nonatomic, copy) NSMutableDictionary *options;
@property (nonatomic, assign) NSUInteger maxOperations;

@property (nonatomic, retain) AVCustomCapture *camera;
- (void)start;
- (void)stop;
- (void)swapCamera;

@end
