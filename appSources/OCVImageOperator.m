#import <UIKit/UIButton.h>

#import "common.h"
#import "utils.h"

#import "OCVImageOperator.h"

#include "operateImage.h"

@implementation OCVImageOperator

- (id)init {
	if ((self = [super init])) {
		_myCam = [[AVCustomCapture alloc] initWithDelegate:self];
		_myCam.videoOrientation = AVCaptureVideoOrientationPortrait;
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

- (void)getCGImage:(CGImageRef)cgImage {
	if (!cgImage) { return; }

	// Create an image object from the Quartz image
	UIImage *gImage = [UIImage imageWithCGImage:cgImage];
	UIImage *gImage2 = operateImage(gImage, nil, _options);
	[self setViewImage:gImage2];
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

- (void)start {
	dispatch_queue_t q = dispatch_queue_create("com.uroboro.operator.start", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(q, ^{
		[_myCam start];
	});
	dispatch_release(q);
}
- (void)stop {
	dispatch_queue_t q = dispatch_queue_create("com.uroboro.operator.stop", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(q, ^{
		[_myCam stop];
	});
	dispatch_release(q);
}
- (void)swapCamera {
	dispatch_queue_t q = dispatch_queue_create("com.uroboro.operator.swap", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(q, ^{
		[_myCam swapCamera];
	});
	dispatch_release(q);
}

@end
