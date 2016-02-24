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
	}

	return self;
}

- (void)dealloc {
	[_image release];

	[super dealloc];
}

- (UIImage *)operateImageCreate:(UIImage *)image {
	UIImage *prevImage = _image;
	_image = [image retain];

	UIImage *r = operateImageCreate(image, prevImage, _options);
	[prevImage release];

	return r;
}

- (void)updateView {
	if (!_image) { return; }

	UIImage *image = [self operateImageCreate:_image];
	[self setViewImage:image];
}

- (void)getCGImage:(CGImageRef)imageRef {
	if (!imageRef) { return; }

	CGImageRef imageRef2 = operateImageRefCreate(imageRef, nil, _options);
	if (_options[@"fps"]) {
		[self setFPS:((NSNumber *)_options[@"fps"]).intValue];
	}
	UIImage *image = [[UIImage alloc] initWithCGImage:imageRef2];
	CGImageRelease(imageRef2);

	[self setViewImage:image];

	[image release];
}

- (void)setViewImage:(UIImage *)image {
	if (!image) { return; }

	dispatch_async(dispatch_get_main_queue(), ^{
		UIImage *img = [[UIImage alloc] initWithCGImage:image.CGImage];

		CGRect availableRect = UtilsAvailableScreenRect();
		CGFloat k = img.size.height / img.size.width;
		CGRect f = _view.frame;
		_view.frame = CGRectMake(f.origin.x, f.origin.y, availableRect.size.width, floor(k * availableRect.size.width));

		if ([_view isKindOfClass:[UIImageView class]]) {
			UIImageView *iv = (UIImageView *)_view;
			[iv setImage:img];
		}
		if ([_view isKindOfClass:[UIButton class]]) {
			UIButton *bv = (UIButton *)_view;
			[bv setBackgroundImage:img forState:UIControlStateNormal];
		}

		[img release];
	});
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
