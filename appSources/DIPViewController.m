#import <MobileCoreServices/MobileCoreServices.h>
#include <dlfcn.h>
#include <UIKit/UIGraphics.h>

#import "common.h"
#import "utils.h"
#import "scaleAndRotate.h"

#import "DIPViewController.h"

UIKIT_EXTERN NSString *rvcName(void) {
	return @"DIPViewController";
}

@implementation DIPViewController

- (void)loadView {
	[super loadView];
	self.title = @"Image Processing";

	CGRect availableRect = UtilsAvailableScreenRect();

	_scrollView = [[UIScrollView alloc] initWithFrame:availableRect];
	[_scrollView setBackgroundColor:[UIColor blackColor]];
	_scrollView.contentSize = CGSizeMake(3 * availableRect.size.width, availableRect.size.height);
	_scrollView.pagingEnabled = YES;
	_scrollView.delegate = self;

	_actionButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_actionButton setFrame:availableRect];
	[_actionButton setBackgroundColor:[UIColor darkGrayColor]];
	[_actionButton addTarget:self action:@selector(captureImage:) forControlEvents:UIControlEventTouchUpInside];
	_currentImage = [self previousActionImage];
	if (_currentImage) {
		CGFloat k = floor(_currentImage.size.height / _currentImage.size.width * availableRect.size.width);
		[_actionButton setBackgroundImage:_currentImage forState:UIControlStateNormal];
		[_actionButton setFrame:CGRectMake(0, 0, availableRect.size.width, k)];
	}
	[_scrollView addSubview:_actionButton];

	_configButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_configButton setFrame:CGRectOffset(availableRect, availableRect.size.width, 0)];
	[_configButton setBackgroundColor:[UIColor lightGrayColor]];
	[_configButton addTarget:self action:@selector(processImage:) forControlEvents:UIControlEventTouchUpInside];
	[_scrollView addSubview:_configButton];

	_cameraView = [[UIImageView alloc] initWithFrame:CGRectOffset(availableRect, 2 * availableRect.size.width, 0)];
	[_cameraView setBackgroundColor:[UIColor greenColor]];
	[_scrollView addSubview:_cameraView];

	_imageOperator = [[OCVImageOperator alloc] initWithView:_cameraView];
	_imageOperator1 = [[OCVImageOperator alloc] initWithView:_configButton];
	NSDictionary *options = @{
		@"mode":(0?@"double":@"single"),
		@"operation":@(1),
		@"samples":@(10),
		@"colorDepth":@(2),
		@"kernelSize":@(5)
	};
	_options = [options mutableCopy];
	_imageOperator.options = _options;
	_imageOperator1.options = _options;

	[self.view addSubview:_scrollView];
}

- (void)viewDidLoad {
	[self processImage];
}

- (void)viewDidUnload {
	[_actionButton release];
	_actionButton = nil;

	[_configButton release];
	_configButton = nil;

	[_currentImage release];
	_currentImage = nil;

	[super viewDidUnload];
}

#pragma mark - UIScrollViewDelegate

- (void)scrollViewWillBeginDecelerating:(UIScrollView *)scrollView {
	scrollView.userInteractionEnabled = NO;
}

- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView {
	scrollView.userInteractionEnabled = YES;

	[UIApplication sharedApplication].idleTimerDisabled = NO;
	self.navigationItem.rightBarButtonItem = nil;
	//Run your code on the current page
	int page = scrollView.contentOffset.x / scrollView.frame.size.width;
	//int pages = scrollView.contentSize.width / scrollView.frame.size.width;
	switch (page) {
	case 0:
		self.title = @"Image Processing";
		[_imageOperator stop];
		break;

	case 1:
		self.title = @"Processed image";
		[_imageOperator stop];
		break;

	case 2:
		self.title = @"Camera";
		[_imageOperator start];
		[UIApplication sharedApplication].idleTimerDisabled = YES;
		UIBarButtonItem *item = [[UIBarButtonItem alloc] initWithTitle:@"Swap" style:UIBarButtonItemStylePlain target:self action:@selector(swapCamera)];
		self.navigationItem.rightBarButtonItem = item;
		[item release];
		break;

	default:
		self.title = @"Image Processing";
		[_imageOperator stop];
		break;
	}
}

#pragma mark - Stuff

- (void)swapCamera {
	[_imageOperator swapCamera];
}

- (void)captureImage:(id)sender {
	[self startCameraControllerFromViewController:self usingDelegate:self];
}

- (void)processImage:(id)sender {
	static int operation = 8;
	operation %= _imageOperator.maxOperations;
	operation++;

	_options[@"operation"] = @(operation);
//UIAlert(@"_options", _options.description);
	_imageOperator.options = _options;
	_imageOperator1.options = _options;
	[self processImage];
}

- (UIImage *)previousActionImage {
	return [UIImage imageWithContentsOfFile:UtilsDocumentPathWithName(@"Photo.png")];
}

- (void)setCapturedImage:(UIImage *)image {
	image = scaleAndRotateImage(image);
	_currentImage = image;

	CGRect availableRect = UtilsAvailableScreenRect();
	CGFloat k = floor(image.size.height / image.size.width * availableRect.size.width);

	[_actionButton setFrame:CGRectMake(0, 0, availableRect.size.width, k)];
	[_actionButton setBackgroundImage:image forState:UIControlStateNormal];

	[UIImagePNGRepresentation(image) writeToFile:UtilsDocumentPathWithName(@"Photo.png") atomically:YES];

	[self processImage];
}

- (void)processImage {
	dispatch_queue_t q = dispatch_queue_create("com.uroboro.dip.image.process", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(q, ^{
		dispatch_async(dispatch_get_main_queue(), ^{
			[_configButton setTitle:@"PROCESSING" forState:UIControlStateNormal];
		});

		if (!_currentImage) { UIAlert(@"!_currentImage",nil); return; }

		UIImage *gImage = [_imageOperator1 operateImage:_currentImage];

		if (!gImage) { UIAlert(@"!gImage",nil); return; }

		// UIKIT happens in the main thread/queue
		dispatch_async(dispatch_get_main_queue(), ^{
			CGRect availableRect = UtilsAvailableScreenRect();
			CGFloat k = floor(gImage.size.height / gImage.size.width * availableRect.size.width);

			CGRect f = _configButton.frame;
			[_configButton setFrame:CGRectMake(f.origin.x, f.origin.y, availableRect.size.width, k)];
			[_configButton setBackgroundImage:gImage forState:UIControlStateNormal];
			[_configButton setTitle:nil forState:UIControlStateNormal];
		});
	});
	dispatch_release(q);
}

#pragma mark - Media input controller

- (BOOL)startCameraControllerFromViewController:(UIViewController *)controller
	usingDelegate:(id <UINavigationControllerDelegate,
	UIImagePickerControllerDelegate>) delegate {

	if (![UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera]
			|| (delegate == nil) || (controller == nil)) {
		return NO;
	}


	UIImagePickerController *cameraUI = [[UIImagePickerController alloc] init];
	cameraUI.sourceType = UIImagePickerControllerSourceTypeCamera;

	// Displays a control that allows the user to choose picture or
	// movie capture, if both are available:
	cameraUI.mediaTypes =
		[UIImagePickerController availableMediaTypesForSourceType:
			UIImagePickerControllerSourceTypeCamera];

	// Hides the controls for moving & scaling pictures, or for
	// trimming movies. To instead show the controls, use YES.
	cameraUI.allowsEditing = NO;

	cameraUI.delegate = delegate;

	[controller presentModalViewController:cameraUI animated:YES];
	return YES;
}

#pragma mark - UIImagePickerControllerDelegate

// For responding to the user tapping Cancel.
- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker {
	[self dismissModalViewControllerAnimated:YES];
	[picker release];
}

- (void)imagePickerController:(UIImagePickerController *)picker
		didFinishPickingImage:(UIImage *)image
		editingInfo:(NSDictionary *)editingInfo {
// Available in iOS 2.0 - 3.0.
	NSMutableDictionary *_editingInfo = [NSMutableDictionary dictionaryWithDictionary:editingInfo];
	[_editingInfo setObject:image forKey:UIImagePickerControllerOriginalImage];
	[_editingInfo setObject:(NSString *)kUTTypeImage forKey:UIImagePickerControllerMediaType];
	[self imagePickerController:picker didFinishPickingMediaWithInfo:_editingInfo];
}

// For responding to the user accepting a newly-captured picture or movie
- (void)imagePickerController:(UIImagePickerController *)picker
			didFinishPickingMediaWithInfo:(NSDictionary *)info {

	NSString *mediaType = [info objectForKey:UIImagePickerControllerMediaType];

	// Handle a still image capture
	if (CFStringCompare((CFStringRef)mediaType, kUTTypeImage, 0) == kCFCompareEqualTo) {
		UIImage *originalImage = (UIImage *) [info objectForKey:
			UIImagePickerControllerOriginalImage];
		[self setCapturedImage:originalImage];
	}

	[self imagePickerControllerDidCancel:picker];
}

@end
