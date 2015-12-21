#import <MobileCoreServices/MobileCoreServices.h>
#import "AVCustomCapture.h"

#import "common.h"

CGImageRef createCGImageRefFromSampleBuffer(CMSampleBufferRef sampleBuffer, BOOL mirrored);

AVCaptureDeviceInput *getCaptureDeviceInputWithDevicePosition(AVCaptureDevicePosition captureDevicePosition);
AVCaptureVideoDataOutput *createCaptureDeviceOutputWithDelegate(id<AVCaptureVideoDataOutputSampleBufferDelegate> delegate);
AVCaptureSession *createCaptureSession(id<AVCaptureVideoDataOutputSampleBufferDelegate> delegate, AVCaptureDevicePosition captureDevicePosition);

@implementation AVCustomCapture

- (id)initWithDelegate:(id<AVCustomCaptureDelegate>)delegate {
	if ((self = [super init])) {
		_delegate = delegate;
		_videoOrientation = AVCaptureVideoOrientationPortrait;
		_devicePosition = AVCaptureDevicePositionBack;
		_session = createCaptureSession(self, _devicePosition);
	}
	return self;
}

- (void)setFPS:(int32_t)fps {
	if (kCFCoreFoundationVersionNumber <= 700) {
		AVCaptureVideoDataOutput *output = (AVCaptureVideoDataOutput *)_session.outputs[0];
		output.minFrameDuration = CMTimeMake(1, fps);
	} else {
		if (_connection) {
			[_connection setVideoMinFrameDuration:CMTimeMake(1, fps)];
		}
	}
}

- (void)setVideoOrientation:(AVCaptureVideoOrientation)videoOrientation {
	_videoOrientation = videoOrientation;
	if (_connection) {
		[_connection setVideoOrientation:videoOrientation];
	}
}

- (void)setDevicePosition:(AVCaptureDevicePosition)captureDevicePosition {
	_devicePosition = captureDevicePosition;
	AVCaptureDeviceInput *input = getCaptureDeviceInputWithDevicePosition(_devicePosition);
	if (!input) {
		UIAlert(@"no input", nil);
		return;
	}

	AVCaptureDeviceInput *input0 = (AVCaptureDeviceInput *)_session.inputs[0];
	[_session removeInput:input0];
	[_session addInput:input];
}

#pragma mark - Controls

- (void)start {
	// Start the session running to start the flow of data
	if (!_session.running) {
		[_session startRunning];
	}
}
- (void)stop {
	if (_session.running) {
		[_session stopRunning];
	}
}
- (void)swapCamera {
	switch (_devicePosition) {
	case AVCaptureDevicePositionBack:
		[self setDevicePosition:AVCaptureDevicePositionFront];
		break;

	case AVCaptureDevicePositionFront:
		[self setDevicePosition:AVCaptureDevicePositionBack];
		break;

	default:
		[self setDevicePosition:AVCaptureDevicePositionBack];
		break;
	}
}

#pragma mark - AVCaptureVideoDataOutputSampleBufferDelegate

// Delegate routine that is called when a sample buffer was written
- (void)captureOutput:(AVCaptureOutput *)captureOutput
		didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
		fromConnection:(AVCaptureConnection *)connection
{
	_connection = connection;
	[connection setVideoOrientation:_videoOrientation];

	// Create a UIImage from the sample buffer data
	BOOL mirrored = (_devicePosition == AVCaptureDevicePositionBack)? NO : YES;
	CGImageRef imageRef = createCGImageRefFromSampleBuffer(sampleBuffer, mirrored);

	// Use the image
	if (_delegate) {
		[_delegate getCGImage:imageRef];
	}

	// Release the Quartz image
	CGImageRelease(imageRef);
}

@end


AVCaptureDeviceInput *getCaptureDeviceInputWithDevicePosition(AVCaptureDevicePosition captureDevicePosition) {
	// Find a suitable AVCaptureDevice
	__block AVCaptureDevice *device = nil;
	NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
	[devices enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
		AVCaptureDevice *dev = (AVCaptureDevice *)obj;
		if (dev.position == captureDevicePosition) {
			device = dev;
			*stop = YES;
		}
	}];

	// Create a device input with the device and add it to the session.
	NSError *error = nil;
	AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice:device error:&error];
	if (error) {
		// Handling the error appropriately.
		UIAlert(@"AVCaptureDeviceInput error", error.description);
	}

	return input;
}

AVCaptureVideoDataOutput *createCaptureDeviceOutputWithDelegate(id<AVCaptureVideoDataOutputSampleBufferDelegate> delegate) {
	// Create a VideoDataOutput
	AVCaptureVideoDataOutput *output = [AVCaptureVideoDataOutput new];

	// Configure your output.
	dispatch_queue_t queue = dispatch_queue_create("com.avcapturesession.private.q", NULL);
	[output setSampleBufferDelegate:delegate queue:queue];
	dispatch_release(queue);

	// Specify the pixel format
	output.videoSettings =
				[NSDictionary dictionaryWithObject:
					[NSNumber numberWithInt:kCVPixelFormatType_32BGRA]
					forKey:(id)kCVPixelBufferPixelFormatTypeKey];

	output.minFrameDuration = CMTimeMake(1, 30);

	return output;
}

// Create and configure a capture session and start it running
AVCaptureSession *createCaptureSession(id<AVCaptureVideoDataOutputSampleBufferDelegate> delegate, AVCaptureDevicePosition captureDevicePosition) {
	AVCaptureSession *session = [AVCaptureSession new];
	session.sessionPreset = AVCaptureSessionPresetMedium;
	AVCaptureDeviceInput *input = getCaptureDeviceInputWithDevicePosition(captureDevicePosition);
	if (!input) {
		UIAlert(@"no input", nil);
		return nil;
	}
	[session addInput:input];

	AVCaptureVideoDataOutput *output = createCaptureDeviceOutputWithDelegate(delegate);
	if (!output) {
		UIAlert(@"no output", nil);
		return nil;
	}
	[session addOutput:output];
	[output release];

	return session;
}


CGImageRef createMirrorImage(CGImageRef imageRef) {
	size_t width = CGImageGetWidth(imageRef);
	size_t height = CGImageGetHeight(imageRef);

	UIGraphicsBeginImageContext(CGSizeMake(width, height));
	CGContextRef context = UIGraphicsGetCurrentContext();

	CGContextTranslateCTM(context, width, height);
	CGContextRotateCTM(context, M_PI);
//	CGContextScaleCTM(context, 1, -1);

	CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
	CGImageRef output = CGBitmapContextCreateImage(context);
	CGContextRelease(context);

	return output;
}

// Create a UIImage from sample buffer data
CGImageRef createCGImageRefFromSampleBuffer(CMSampleBufferRef sampleBuffer, BOOL mirrored) {
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

	if (mirrored) {
		CGImageRef image = createMirrorImage(quartzImage);
		CGImageRelease(quartzImage);
		quartzImage = image;
	}

	return quartzImage;
}
