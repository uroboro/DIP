#import <UIKit/UIButton.h>

#import "common.h"
#import "utils.h"

#import "OCVImageOperator.h"

#include "operateImage.h"

@implementation OCVImageOperator

- (id)init {
	if ((self = [super init])) {
		_camera = [[AVCustomCapture alloc] initWithDelegate:self];
		_camera.videoOrientation = AVCaptureVideoOrientationPortrait;
	}
	return self;
}

- (id)initWithView:(UIView *)view {
	if ((self = [self init])) {
		_view = view;
		#if PREVIEW_LAYER
		_previewLayer = [AVCaptureVideoPreviewLayer layerWithSession:_camera.session];
		_previewLayer.frame = _view.bounds;
		[_view.layer addSublayer:_previewLayer];
		#endif
	}

	return self;
}

- (void)dealloc {
	[super dealloc];
}

- (void)getCGImage:(CGImageRef)imageRef {
	operateImageProcessImageAndUpdateView(imageRef, _view, _options);
	if (_options[@"fps"]) {
		[self setFPS:((NSNumber *)_options[@"fps"]).intValue];
	}
}

#pragma mark - Camera controls

- (void)start {
	dispatch_queue_t q = dispatch_queue_create("com.uroboro.operator.start", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(q, ^{
		[_camera start];
	});
	dispatch_release(q);
}
- (void)stop {
	dispatch_queue_t q = dispatch_queue_create("com.uroboro.operator.stop", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(q, ^{
		[_camera stop];
	});
	dispatch_release(q);
}
- (void)swapCamera {
	dispatch_queue_t q = dispatch_queue_create("com.uroboro.operator.swap", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(q, ^{
		[_camera stop];
		[_camera swapCamera];
		[_camera start];
	});
	dispatch_release(q);
}

- (void)setFPS:(int32_t)fps {
	dispatch_queue_t q = dispatch_queue_create("com.uroboro.operator.fps", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(q, ^{
		[_camera setFPS:fps];
	});
	dispatch_release(q);
}

@end
