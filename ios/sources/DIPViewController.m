#import <MobileCoreServices/MobileCoreServices.h>
#include <dlfcn.h>
#include <UIKit/UIGraphics.h>

#import "common.h"
#import "utils.h"
#import "ImageUtils.h"

#import "DIPViewController.h"
#import "OCVImageOperator.h"

UIKIT_EXTERN NSString *rvcName(void) {
	return @"DIPViewController";
}

@interface DIPViewController ()

@property (nonatomic, retain) UIScrollView *scrollView;
@property (nonatomic, retain) UIButton *actionButton;
@property (nonatomic, retain) UIImageView *stillShotView;
@property (nonatomic, retain) UIImageView *cameraView;

@property (nonatomic, retain) UIImage *currentImage;
@property (nonatomic, retain) OCVImageOperator *imageOperatorImage;
@property (nonatomic, retain) OCVImageOperator *imageOperatorVideo;

@property (nonatomic, retain) NSMutableDictionary *options;

@end

@implementation DIPViewController

- (void)loadView {
	[super loadView];
	self.title = @"Image Processing";

	CGRect availableRect = UtilsAvailableScreenRect();

	[self.view addSubview:_scrollView = ({
		UIScrollView *scrollView = [[UIScrollView alloc] initWithFrame:availableRect];
		[scrollView setBackgroundColor:[UIColor blackColor]];
		scrollView.contentSize = CGSizeMake(3 * availableRect.size.width, availableRect.size.height);
		scrollView.pagingEnabled = YES;
		scrollView.delegate = self;
		scrollView;
	})];

	[_scrollView addSubview:_actionButton = ({
		UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
		[button setFrame:availableRect];
		[button setBackgroundColor:[UIColor darkGrayColor]];
		[button addTarget:self action:@selector(actionButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
		_currentImage = [self previousStillShotImage];
		if (_currentImage) {
			CGFloat k = floor(_currentImage.size.height / _currentImage.size.width * availableRect.size.width);
			[button setBackgroundImage:_currentImage forState:UIControlStateNormal];
			[button setFrame:CGRectMake(0, 0, availableRect.size.width, k)];
		}
		button;
	})];

	[_scrollView addSubview:_stillShotView = ({
		UIImageView *imageView = [[UIImageView alloc] initWithFrame:CGRectOffset(availableRect, availableRect.size.width, 0)];
		[imageView setBackgroundColor:[UIColor lightGrayColor]];
		imageView;
	})];

	[_scrollView addSubview:_cameraView = ({
		UIImageView *imageView = [[UIImageView alloc] initWithFrame:CGRectOffset(availableRect, 2 * availableRect.size.width, 0)];
		[imageView setBackgroundColor:[UIColor greenColor]];
		imageView;
	})];

	_imageOperatorImage = ({
		OCVImageOperator *imageOperator = [[OCVImageOperator alloc] initWithView:_stillShotView];
		imageOperator.options = [@{@"inputType":@"image",@"floatingValue":@(1)} mutableCopy];
		imageOperator;
	});

	_imageOperatorVideo = ({
		OCVImageOperator *imageOperator = [[OCVImageOperator alloc] initWithView:_cameraView];
		imageOperator.options = [@{@"inputType":@"video",@"fps":@(30),@"floatingValue":@(0.25)} mutableCopy];
		[imageOperator.camera swapCamera];
		imageOperator;
	});

	if(01)[_scrollView addSubview:({
		CGRect frame = CGRectMake(availableRect.size.width / 32, availableRect.size.height * 9 / 10, availableRect.size.width * 15 / 16, 20);
		frame = CGRectOffset(frame, 2 * availableRect.size.width, 0);
		UISlider *slider = [[UISlider alloc] initWithFrame:frame];
		slider.value = 1;
		[slider addTarget:self action:@selector(sliderChanged:) forControlEvents:UIControlEventValueChanged];
		[self sliderChanged:slider];
		slider;
	})];
}

- (void)viewDidLoad {
	[self processImage];
}

- (void)viewDidUnload {
	[_actionButton release];
	_actionButton = nil;

	[_stillShotView release];
	_stillShotView = nil;

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
		[_imageOperatorVideo stop];
		break;

	case 1:
		self.title = @"Processed image";
		[_imageOperatorVideo stop];
		break;

	case 2:
		self.title = [@"Camera" stringByAppendingString:(_imageOperatorVideo.camera.devicePosition == AVCaptureDevicePositionBack) ? @" Back":@" Front"];
		[_imageOperatorVideo start];
		[UIApplication sharedApplication].idleTimerDisabled = YES;
		UIBarButtonItem *item = [[UIBarButtonItem alloc] initWithTitle:@"Swap" style:UIBarButtonItemStylePlain target:self action:@selector(swapButtonPressed:)];
		self.navigationItem.rightBarButtonItem = item;
		[item release];
		break;

	default:
		self.title = @"Image Processing";
		[_imageOperatorVideo stop];
		break;
	}
}

#pragma mark - Button selectors

- (void)actionButtonPressed:(id)sender {
	[self captureImage];
}
- (void)swapButtonPressed:(id)sender {
	[self swapCamera];
}

#pragma mark - Slider selectors

- (void)sliderChanged:(UISlider *)sender {
	[_imageOperatorVideo.options setObject:@(sender.value) forKey:@"floatingValue"];
}

#pragma mark - Stuff

- (void)swapCamera {
	self.title = [@"Camera" stringByAppendingString:(_imageOperatorVideo.camera.devicePosition != AVCaptureDevicePositionBack) ? @" Back":@" Front"];
	[_imageOperatorVideo swapCamera];
}

- (void)captureImage {
	[self startCameraControllerFromViewController:self usingDelegate:self];
}

- (UIImage *)previousStillShotImage {
	return [UIImage imageWithContentsOfFile:UtilsDocumentPathWithName(@"Photo.png")];
}

- (void)setCapturedImage:(UIImage *)image {
	[_currentImage release];
	_currentImage = UIImageCreateApplyingMetadata(image);

	CGRect availableRect = UtilsAvailableScreenRect();
	CGFloat k = floor(_currentImage.size.height / _currentImage.size.width * availableRect.size.width);

	[_actionButton setFrame:CGRectMake(0, 0, availableRect.size.width, k)];
	[_actionButton setBackgroundImage:_currentImage forState:UIControlStateNormal];

	[UIImagePNGRepresentation(_currentImage) writeToFile:UtilsDocumentPathWithName(@"Photo.png") atomically:YES];

	[self processImage];
}

- (void)processImage {
	dispatch_queue_t q = dispatch_queue_create("com.uroboro.dip.image.process", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(q, ^{
		if (!_currentImage) { UIAlert(@"!_currentImage",nil); return; }
		[_imageOperatorImage getCGImage:_currentImage.CGImage];
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

	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[controller presentModalViewController:cameraUI animated:YES];
	#pragma clang diagnostic pop
	return YES;
}

#pragma mark - UIImagePickerControllerDelegate

// For responding to the user tapping Cancel.
- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[self dismissModalViewControllerAnimated:YES];
	#pragma clang diagnostic pop
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
	if ([mediaType isEqualToString:(NSString *)kUTTypeImage]) {
		UIImage *originalImage = (UIImage *)[info objectForKey:UIImagePickerControllerOriginalImage];
		[self setCapturedImage:originalImage];
	}

	[self imagePickerControllerDidCancel:picker];
}

@end
