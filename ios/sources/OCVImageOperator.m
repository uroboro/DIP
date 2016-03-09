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
	if (!imageRef) { return; }

	#define SCALE 0
	#if SCALE
		float floatingValue = options[@"floatingValue"] ? ((NSNumber *)options[@"floatingValue"]).floatValue : 1;
		NSLog2(([NSString stringWithFormat:@"floatingValue %@", options[@"floatingValue"]]).UTF8String);
		// Scale down input image
		if (floatingValue < 1) { CGImageRef tmp = CGImageCreateScaled(imageRef, floatingValue); if (tmp) { imageRef = tmp; } }
	#endif

	CGImageRef imageRefOut = operateImageRefCreate(imageRef);
	if (!imageRefOut) { present(DBGOutputModeSyslog|0, "no imageRefOut"); return; }

	#if SCALE
		// Scale up output image
		if (floatingValue < 1) { CGImageRef tmp = CGImageCreateScaled(imageRefOut, 1/floatingValue); if (tmp) { CGImageRelease(imageRefOut); imageRefOut = tmp; } }
	#endif

	CGRect availableRect = UtilsAvailableScreenRect();
	CGFloat k = (CGFloat)CGImageGetHeight(imageRefOut) / CGImageGetWidth(imageRefOut);
	_view.frame = CGRectMake(_view.frame.origin.x, _view.frame.origin.y, availableRect.size.width, floor(k * availableRect.size.width));

	UIImage *image = [[UIImage alloc] initWithCGImage:imageRefOut];
	dispatch_async(dispatch_get_main_queue(), ^{
		if ([_view isKindOfClass:[UIImageView class]]) {
			UIImageView *iv = (UIImageView *)_view;
			[iv setImage:image];
		}
		[image release];
	});
	CGImageRelease(imageRefOut);

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
