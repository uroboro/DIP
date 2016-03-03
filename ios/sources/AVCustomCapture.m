#import "AVCustomCapture.h"
#import "ImageUtils.h"

#import "common.h"

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
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	if (kCFCoreFoundationVersionNumber <= 700) {
		AVCaptureVideoDataOutput *output = (AVCaptureVideoDataOutput *)_session.outputs[0];

		output.minFrameDuration = CMTimeMake(1, fps);
	} else if (kCFCoreFoundationVersionNumber <= 800) {
		if (_connection && _connection.isVideoMinFrameDurationSupported) {
			_connection.videoMinFrameDuration = CMTimeMake(1, fps);
		}
	} else {
		AVCaptureDeviceInput *input = (AVCaptureDeviceInput *)_session.inputs[0];
		NSError *error = nil;
		if ([input.device lockForConfiguration:&error]) {
			@try {
				input.device.activeVideoMinFrameDuration = CMTimeMake(1, fps);
			}
			@catch ( NSException *e ) {
			}
			[input.device unlockForConfiguration];
		}
	}
	#pragma clang diagnostic pop
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
	CGImageRef imageRef = CGImageCreateFromSampleBuffer(sampleBuffer);
	if (mirrored) {
		CGImageRef image = CGImageCreateMirrorImage(imageRef);
		CGImageRelease(imageRef);
		imageRef = image;
	}
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
	output.videoSettings = @{ (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA)};

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
