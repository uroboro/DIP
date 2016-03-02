#include "messages.h"

#include "operateImage.h"
#include "UIImage+IplImage.h"
#import "utils.h"

#include "ocv_hand.h"

IplImage *ocv_handSpriteCreate(char *path) {
	UIImage *uiImage = [[UIImage alloc] initWithContentsOfFile:UtilsResourcePathWithName(@(path))];
	IplImage *sprite = IplImageFromCGImage(uiImage.CGImage);
	[uiImage release];
	return sprite;
}

CGImageRef operateImageRefCreate(CGImageRef imageRef0, CGImageRef imageRef1, NSMutableDictionary *options) {
	if (!imageRef0) { present(1, "!imageRef0"); return nil; }
	NSLog2("operating");

	#define SCALE 0
	#if SCALE
		float floatingValue = options[@"floatingValue"] ? ((NSNumber *)options[@"floatingValue"]).floatValue : 1;
		NSLog2(([NSString stringWithFormat:@"floatingValue %@", options[@"floatingValue"]]).UTF8String);
		// Scale down input image
		if (floatingValue < 1) { CGImageRef tmp = CreateScaledCGImageFromCGImage(imageRef0, floatingValue); if (tmp) { imageRef0 = tmp; } }
	#endif

	IplImage *iplInput = IplImageFromCGImage(imageRef0);
	if (!iplInput) { present(1, "!iplInput"); return nil; }

	IplImage *iplOutput = cvCloneImage(iplInput);

	//test orientation handling
	//if ([options[@"inputType"] isEqualToString:@"image"]) { cvFlip(iplImage, NULL, 1); }

	ocv_handAnalysis(iplInput, iplOutput);
	cvReleaseImage(&iplInput);

	CGImageRef imageRefOut = CGImageFromIplImage(iplOutput);
	cvReleaseImage(&iplOutput);

	#if SCALE
		// Scale up output image
		if (floatingValue < 1) { CGImageRef tmp = CreateScaledCGImageFromCGImage(imageRefOut, 1/floatingValue); if (tmp) { CGImageRelease(imageRefOut); imageRefOut = tmp; } }
	#endif

	options[@"fps"] = @(5);

	return imageRefOut;
}

UIImage *operateImageCreate(UIImage *image0, UIImage *image1, NSMutableDictionary *options) {
	CGImageRef imageRef0 = image0.CGImage;
	CGImageRef imageRef1 = image1.CGImage;

	CGImageRef imageRefOut = operateImageRefCreate(imageRef0, imageRef1, options);

	UIImage *uiImage = [[UIImage alloc] initWithCGImage:imageRefOut];
	CGImageRelease(imageRefOut);

	return uiImage;
}
