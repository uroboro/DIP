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

		// Preview Layer
		_customPreviewLayer = [CALayer layer];
		_customPreviewLayer.frame = CGRectMake(0, 0, _view.frame.size.width, _view.frame.size.height);
		[_view.layer addSublayer:_customPreviewLayer];
	}

	return self;
}

- (void)dealloc {
	[_image release];

	[super dealloc];
}

- (NSUInteger)maxOperations {
	_maxOperations = maxOperations();
	return _maxOperations;
}

- (UIImage *)operateImage:(UIImage *)image {
	UIImage *prevImage = _image;
	_image = [image retain];
	UIImage *r = operateImage(image, prevImage, _options);
	[prevImage release];

	return r;
}

- (void)updateView {
	if (!_image) { return; }

	UIImage *gImage = [self operateImage:_image];
	[self setViewImage:gImage];
}

- (void)getCGImage:(CGImageRef)imageRef {
	if (!imageRef) { return; }

	size_t width = CGImageGetWidth(imageRef);
	size_t height = CGImageGetHeight(imageRef);
	CGRect availableRect = UtilsAvailableScreenRect();
	CGFloat k = floor((CGFloat)height / width * availableRect.size.width);
	_customPreviewLayer.frame = CGRectMake(0, 0, availableRect.size.width, k);

	CGImageRef imageRef2 = operateImageRef(imageRef, nil, _options);
	dispatch_sync(dispatch_get_main_queue(), ^{
		_customPreviewLayer.contents = (__bridge id)imageRef2;
	});
	CGImageRelease(imageRef2);
}

- (void)setViewImage:(UIImage *)image {
	dispatch_async(dispatch_get_main_queue(), ^{
		CGRect availableRect = UtilsAvailableScreenRect();
		CGFloat k = floor(image.size.height / image.size.width * availableRect.size.width);

		CGRect f = _view.frame;
		[_view setFrame:CGRectMake(f.origin.x, f.origin.y, availableRect.size.width, k)];
		if ([_view isKindOfClass:[UIImageView class]]) {
			UIImageView *iv = (UIImageView *)_view;
			iv.image = image;
		}
		if ([_view isKindOfClass:[UIButton class]]) {
			UIButton *bv = (UIButton *)_view;
			[bv setBackgroundImage:image forState:UIControlStateNormal];
		}
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

@end
