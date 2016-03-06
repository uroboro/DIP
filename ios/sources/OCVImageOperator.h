#import <UIKit/UIView.h>
#import <UIKit/UIImage.h>

#import "AVCustomCapture.h"

@interface OCVImageOperator : NSObject <AVCustomCaptureDelegate> {
}
@property (nonatomic, assign) UIView *view;
- (id)initWithView:(UIView *)view;

@property (nonatomic, retain) NSMutableDictionary *options;

@property (nonatomic, retain) AVCustomCapture *camera;
@property (nonatomic, retain) AVCaptureVideoPreviewLayer *previewLayer;

- (void)start;
- (void)stop;
- (void)swapCamera;
- (void)setFPS:(int32_t)fps;

@end
