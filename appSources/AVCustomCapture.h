#import <AVFoundation/AVFoundation.h>

@protocol AVCustomCaptureDelegate
- (void)getCGImage:(CGImageRef)cgImage;
@end

@interface AVCustomCapture : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

- (id)initWithDelegate:(id<AVCustomCaptureDelegate>)delegate;
@property (nonatomic, assign) id<AVCustomCaptureDelegate> delegate;

@property (nonatomic, assign) AVCaptureVideoOrientation videoOrientation;
@property (nonatomic, assign) AVCaptureDevicePosition devicePosition;

- (void)start;
- (void)stop;
- (void)swapCamera;
//- (void)getCGShot:(CGImageRef)cgImage;

@property (nonatomic, retain) AVCaptureSession *session;
@property (nonatomic, assign) AVCaptureConnection *connection;

@end
