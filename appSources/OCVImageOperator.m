#include <dlfcn.h>
#import <UIKit/UIButton.h>

#import "common.h"
#import "utils.h"

#import "OCVImageOperator.h"

dispatch_source_t monitorUpdatesToFile(const char* filename, dispatch_block_t event_handler, dispatch_block_t cancel_handler);

@implementation OCVImageOperator

- (id)initWithView:(UIView *)view {
	if ((self = [self init])) {
		_view = view;
	}

	return self;
}

- (id)init {
	if ((self = [super init])) {
		const char *dylibPath = UtilsResourcePathWithName(@"dip.dylib").UTF8String;
		_handle = dlopen(dylibPath, RTLD_NOW);
		if (!_handle) { UIAlert(@"!dylib", ([NSString stringWithFormat:@"%s", dylibPath])); return self; }
#if 0
		_source = monitorUpdatesToFile(dylibPath, ^{
			dlclose(_handle);
			_handle = dlopen(dylibPath, RTLD_NOW);
			[self updateView];
		}, ^{});
#endif
	}
	return self;
}

- (void)dealloc {
	dispatch_source_cancel(_source);
	dlclose(_handle);
	[_image release];

	[super dealloc];
}

- (UIImage *)operateImage:(UIImage *)image {
	[_image release];
	_image = [image retain];

	static UIImage *(*operateImage)(UIImage *, UIImage *) = NULL;
	if (!operateImage) {
		operateImage = (UIImage *(*)(UIImage *, UIImage *))dlsym(_handle, "operateImage");
	}
	if (!operateImage) { UIAlert(@"!\"operateImage\" couldn't be found.\n",nil); return nil; }

	return operateImage(image, nil);
}

- (void)updateView {
	if (!_image) { return; }

	UIImage *gImage = [self operateImage:_image];

	dispatch_async(dispatch_get_main_queue(), ^{
		CGRect availableRect = UtilsAvailableScreenRect();
		CGFloat k = floor(gImage.size.height / gImage.size.width * availableRect.size.width);

		[_view setFrame:CGRectMake(availableRect.size.width, 0, availableRect.size.width, k)];
		[(UIButton *)_view setBackgroundImage:gImage forState:UIControlStateNormal];
	});
}

@end

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
