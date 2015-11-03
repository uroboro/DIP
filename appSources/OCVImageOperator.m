#include <dlfcn.h>
#import <UIKit/UIButton.h>

#import "common.h"
#import "utils.h"
#import "scaleAndRotate.h"

#import "OCVImageOperator.h"

#include <opencv2/imgproc/imgproc_c.h>

#if USE_DYLIB
dispatch_source_t monitorUpdatesToFile(const char* filename, dispatch_block_t event_handler, dispatch_block_t cancel_handler);
static BOOL _update = YES;
#else
#include "operateImage.h"
#endif

@implementation OCVImageOperator

- (id)init {
	if ((self = [super init])) {
#if USE_DYLIB
		const char *dylibPath = UtilsResourcePathWithName(@"dip.dylib").UTF8String;
		_handle = dlopen(dylibPath, RTLD_NOW);
		if (!_handle) { UIAlert(@"!dylib", ([NSString stringWithFormat:@"%s", dylibPath])); return self; }

		_source = monitorUpdatesToFile(dylibPath, ^{
			if (_update) {
				_update = NO;

				dlclose(_handle);

				dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC),
					dispatch_get_main_queue(), ^(void) {

					UIAlert(@"updating dylib",nil);
					const char *dylibPath = UtilsResourcePathWithName(@"dip.dylib").UTF8String;
					_handle = dlopen(dylibPath, RTLD_NOW);
					if (!_handle) { UIAlert(@"!dylib", ([NSString stringWithFormat:@"%s", dylibPath])); return; }

					[self updateView];

					_update = YES;
				});
			}
		}, ^{
			dlclose(_handle);
		});
#endif
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
#if USE_DYLIB
	if (_source) dispatch_source_cancel(_source);
	if (_handle) dlclose(_handle);
#endif
	[_image release];

	[super dealloc];
}

- (NSUInteger)maxOperations {
#if USE_DYLIB
	NSUInteger (*maxOperations)(void) = NULL;
	maxOperations = (NSUInteger (*)(void))dlsym(_handle, "maxOperations");
	if (!maxOperations) { UIAlert(@"!\"maxOperations\" couldn't be found.",nil); return 1; }
#endif
	_maxOperations = maxOperations();
	return _maxOperations;
}

- (UIImage *)operateImage:(UIImage *)image {
	UIImage *prevImage = _image;
	_image = [image retain];
#if USE_DYLIB
	UIImage *(*operateImage)(UIImage *, UIImage *, NSMutableDictionary *) = NULL;
	operateImage = (UIImage *(*)(UIImage *, UIImage *, NSMutableDictionary *))dlsym(_handle, "operateImage");
	if (!operateImage) { UIAlert(@"!\"operateImage\" couldn't be found.",nil); return nil; }
#endif
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
#if USE_DYLIB
	UIImage *(*operateImage)(UIImage *, UIImage *, NSMutableDictionary *) = NULL;
	operateImage = (UIImage *(*)(UIImage *, UIImage *, NSMutableDictionary *))dlsym(_handle, "operateImage");
	if (!operateImage) { UIAlert(@"!\"operateImage\" couldn't be found.",nil); return; }
#endif
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

#if USE_DYLIB
dispatch_source_t monitorUpdatesToFile(const char* filename, dispatch_block_t event_handler, dispatch_block_t cancel_handler) {
	int fd = open(filename, O_EVTONLY);
	if (fd == -1)
		return NULL;
 
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	dispatch_source_t source = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE,
		fd, DISPATCH_VNODE_WRITE, queue);

	if (source) {
		// Copy the filename for later use.
		int length = strlen(filename);
		char* newString = (char*)malloc(length + 1);
		newString = strcpy(newString, filename);
		dispatch_set_context(source, newString);
 
		// Install the event handler to process the name change
		dispatch_source_set_event_handler(source, ^{
			event_handler();
		});
 
		// Install a cancellation handler to free the descriptor
		dispatch_source_set_cancel_handler(source, ^{
			cancel_handler();
			close(fd);
		});
 
		// Start processing events.
		dispatch_resume(source);
	} else {
		close(fd);
	}
 
	return source;
}
#endif