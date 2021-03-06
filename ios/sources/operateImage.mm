#include "messages.h"

#include "operateImage.h"
#include "UIImage+OpenCV.h"
#import "utils.h"

#include "ocv_hand.h"

IplImage *ocv_handSpriteCreate(char *path) {
	UIImage *uiImage = [[UIImage alloc] initWithContentsOfFile:UtilsResourcePathWithName(@(path))];
	IplImage *sprite = uiImage.iplImage;
	[uiImage release];
	return sprite;
}

CGImageRef operateImageRefCreate(CGImageRef imageRef) {
	if (!imageRef) { present(1, "!imageRef0"); return nil; }
	NSLog2("operating");

	CGImageRef imageRefOut = NULL;
	int tries = 0;
	try {
#define USE_IPLIMAGE 01
#if USE_IPLIMAGE
	IplImage *iplInput = IplImageFromCGImage(imageRef);
	if (!iplInput) { present(1, "!iplInput"); return nil; }

	IplImage *iplOutput = cvCloneImage(iplInput);
	ocv_handAnalysis(iplInput, iplOutput);

	imageRefOut = CGImageFromIplImage(iplOutput);
	cvReleaseImage(&iplInput);
	cvReleaseImage(&iplOutput);
#else
	cv::Mat image;
	CVMatFromCGImage(imageRef, image);

	cv::Mat output;
	ocv_handAnalysisMat(image, output);

	imageRefOut = CGImageFromCVMat(output);
#endif
	} catch (cv::Exception& e) {
		if (tries == 0) {
			tries++;
			const char* err_msg = e.what();
			NSString * m = [NSString stringWithFormat:@"OpenCV exception caught: %s", err_msg];
			UIAlert(@"Exception", m);
		}
	}
	return imageRefOut;
}

void operateImageProcessImageAndUpdateView(CGImageRef imageRef, UIImageView *imageView, NSDictionary *options) {
	if (!imageRef) { present(DBGOutputModeSyslog|0, "!imageRef"); return; }

	#define SCALE 0
	#if SCALE
		float floatingValue = options[@"floatingValue"] ? ((NSNumber *)options[@"floatingValue"]).floatValue : 1;
		present(DBGOutputModeSyslog|0, "floatingValue %f", floatingValue);
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
	imageView.frame = CGRectMake(imageView.frame.origin.x, imageView.frame.origin.y, availableRect.size.width, floor(k * availableRect.size.width));

	UIImage *image = [[UIImage alloc] initWithCGImage:imageRefOut];
	dispatch_async(dispatch_get_main_queue(), ^{
		[imageView setImage:image];
		[UIImagePNGRepresentation(image) writeToFile:UtilsDocumentPathWithName(@"Photo_Out.png") atomically:YES];
		[image release];
	});
	CGImageRelease(imageRefOut);
}
