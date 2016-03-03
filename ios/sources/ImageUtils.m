#import <UIKit/UIImage.h>
#import <UIKit/UIGraphics.h>
#import <AVFoundation/AVFoundation.h>

// http://stackoverflow.com/a/20611860/1429562
UIImage *UIImageCreateApplyingMetadata(UIImage *image) {
	int kMaxResolution = 640;

	CGImageRef imgRef = image.CGImage;

	CGFloat width = CGImageGetWidth(imgRef);
	CGFloat height = CGImageGetHeight(imgRef);

	CGAffineTransform transform = CGAffineTransformIdentity;
	CGRect bounds = CGRectMake(0, 0, width, height);
	if (width > kMaxResolution || height > kMaxResolution) {
		CGFloat ratio = width/height;
		if (ratio > 1) {
			bounds.size.width = kMaxResolution;
			bounds.size.height = bounds.size.width / ratio;
		}
		else {
			bounds.size.height = kMaxResolution;
			bounds.size.width = bounds.size.height * ratio;
		}
	}

	CGFloat scaleRatio = bounds.size.width / width;
	CGSize imageSize = CGSizeMake(CGImageGetWidth(imgRef), CGImageGetHeight(imgRef));
	CGFloat boundHeight;
	UIImageOrientation orient = image.imageOrientation;
	switch (orient) {

		case UIImageOrientationUp: //EXIF = 1
			transform = CGAffineTransformIdentity;
			break;

		case UIImageOrientationUpMirrored: //EXIF = 2
			transform = CGAffineTransformMakeTranslation(imageSize.width, 0.0);
			transform = CGAffineTransformScale(transform, -1.0, 1.0);
			break;

		case UIImageOrientationDown: //EXIF = 3
			transform = CGAffineTransformMakeTranslation(imageSize.width, imageSize.height);
			transform = CGAffineTransformRotate(transform, M_PI);
			break;

		case UIImageOrientationDownMirrored: //EXIF = 4
			transform = CGAffineTransformMakeTranslation(0.0, imageSize.height);
			transform = CGAffineTransformScale(transform, 1.0, -1.0);
			break;

		case UIImageOrientationLeftMirrored: //EXIF = 5
			boundHeight = bounds.size.height;
			bounds.size.height = bounds.size.width;
			bounds.size.width = boundHeight;
			transform = CGAffineTransformMakeTranslation(imageSize.height, imageSize.width);
			transform = CGAffineTransformScale(transform, -1.0, 1.0);
			transform = CGAffineTransformRotate(transform, 3.0 * M_PI / 2.0);
			break;

		case UIImageOrientationLeft: //EXIF = 6
			boundHeight = bounds.size.height;
			bounds.size.height = bounds.size.width;
			bounds.size.width = boundHeight;
			transform = CGAffineTransformMakeTranslation(0.0, imageSize.width);
			transform = CGAffineTransformRotate(transform, 3.0 * M_PI / 2.0);
			break;

		case UIImageOrientationRightMirrored: //EXIF = 7
			boundHeight = bounds.size.height;
			bounds.size.height = bounds.size.width;
			bounds.size.width = boundHeight;
			transform = CGAffineTransformMakeScale(-1.0, 1.0);
			transform = CGAffineTransformRotate(transform, M_PI / 2.0);
			break;

		case UIImageOrientationRight: //EXIF = 8
			boundHeight = bounds.size.height;
			bounds.size.height = bounds.size.width;
			bounds.size.width = boundHeight;
			transform = CGAffineTransformMakeTranslation(imageSize.height, 0.0);
			transform = CGAffineTransformRotate(transform, M_PI / 2.0);
			break;

		default:
			[NSException raise:NSInternalInconsistencyException format:@"Invalid image orientation"];

	}

	UIGraphicsBeginImageContext(bounds.size);

	CGContextRef context = UIGraphicsGetCurrentContext();

	if (orient == UIImageOrientationRight || orient == UIImageOrientationLeft) {
		CGContextScaleCTM(context, -scaleRatio, scaleRatio);
		CGContextTranslateCTM(context, -height, 0);
	} else {
		CGContextScaleCTM(context, scaleRatio, -scaleRatio);
		CGContextTranslateCTM(context, 0, -height);
	}

	CGContextConcatCTM(context, transform);

	CGContextDrawImage(context, CGRectMake(0, 0, width, height), imgRef);
	//UIImage *imageCopy = UIGraphicsGetImageFromCurrentImageContext();
	CGImageRef quartzImage = CGBitmapContextCreateImage(context);
	UIGraphicsEndImageContext();

	UIImage *imageCopy = [[UIImage alloc] initWithCGImage:quartzImage];
	CGImageRelease(quartzImage);

	return imageCopy;
}

CGImageRef CGImageCreateMirrorImage(CGImageRef imageRef) {
	size_t width = CGImageGetWidth(imageRef);
	size_t height = CGImageGetHeight(imageRef);

	UIGraphicsBeginImageContext(CGSizeMake(width, height));
	CGContextRef context = UIGraphicsGetCurrentContext();

	CGContextTranslateCTM(context, width, height);
	CGContextRotateCTM(context, M_PI);
	//CGContextScaleCTM(context, 1, -1);

	CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
	CGImageRef output = CGBitmapContextCreateImage(context);
	CGContextRelease(context);

	return output;
}

CGImageRef CGImageCreateScaled(CGImageRef image, CGFloat scale) {
	int width = CGImageGetWidth(image) * scale;
	int height = CGImageGetHeight(image) * scale;

	int bitmapBytesPerRow   = (width * 4);
	int bitmapByteCount     = (bitmapBytesPerRow * height);

	void *bitmapData = malloc(bitmapByteCount);
	if (bitmapData == NULL) {
		return nil;
	}

	CGColorSpaceRef colorspace = CGImageGetColorSpace(image);
	CGContextRef context = CGBitmapContextCreate(bitmapData, width, height, 8,
		bitmapBytesPerRow, colorspace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
	CGColorSpaceRelease(colorspace);

	if (context == NULL) {
		return nil;
	}

	CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);

	CGImageRef imgRef = CGBitmapContextCreateImage(context);
	CGContextRelease(context);
	free(bitmapData);

	return imgRef;
}

// Create a CGImageRef from sample buffer data
CGImageRef CGImageCreateFromSampleBuffer(CMSampleBufferRef sampleBuffer) {
	// Get a CMSampleBuffer's Core Video image buffer for the media data
	CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
	// Lock the base address of the pixel buffer
	CVPixelBufferLockBaseAddress(imageBuffer, 0);

	// Get the number of bytes per row for the pixel buffer
	void *baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);

	// Get the number of bytes per row for the pixel buffer
	size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
	// Get the pixel buffer width and height
	size_t width = CVPixelBufferGetWidth(imageBuffer);
	size_t height = CVPixelBufferGetHeight(imageBuffer);

	// Create a device-dependent RGB color space
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	// Create a bitmap graphics context with the sample buffer data
	CGContextRef context = CGBitmapContextCreate(baseAddress, width, height, 8,
		bytesPerRow, colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
	// Create a Quartz image from the pixel data in the bitmap graphics context
	CGImageRef quartzImage = CGBitmapContextCreateImage(context);
	// Unlock the pixel buffer
	CVPixelBufferUnlockBaseAddress(imageBuffer, 0);

	// Free up the context and color space
	CGContextRelease(context);
	CGColorSpaceRelease(colorSpace);

	return quartzImage;
}
