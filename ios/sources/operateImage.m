#include "messages.h"

#include "operateImage.h"
#include "UIImage+IplImage.h"
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

	IplImage *iplInput = IplImageFromCGImage(imageRef);
	if (!iplInput) { present(1, "!iplInput"); return nil; }

	IplImage *iplOutput = cvCloneImage(iplInput);

	ocv_handAnalysis(iplInput, iplOutput);

	CGImageRef imageRefOut = CGImageFromIplImage(iplOutput);
	cvReleaseImage(&iplInput);
	cvReleaseImage(&iplOutput);

	return imageRefOut;
}
