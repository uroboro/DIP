// Win32_Console.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "operateImage.h"

int _tmain(int argc, char *argv[], char *envp[]) {

	CvCapture *cv_cap = cvCaptureFromCAM(CV_CAP_ANY);
	if (!cv_cap) {
		printf("Could not open camera\n");
		cvWaitKey(0);
		return -1;
	}

	int cam_width = (int)cvGetCaptureProperty(cv_cap, CV_CAP_PROP_FRAME_WIDTH);
	int cam_height = (int)cvGetCaptureProperty(cv_cap, CV_CAP_PROP_FRAME_HEIGHT);
	CvSize cam_size = cvSize(cam_width, cam_height);

	Userdata userdata = getSessionUserdata(cam_size);

	cvNamedWindow(INPUT_WINDOW, 0);
	cvResizeWindow(INPUT_WINDOW, cam_width, cam_height);
	cvNamedWindow(OUTPUT_WINDOW, 0);
	cvResizeWindow(OUTPUT_WINDOW, cam_width, cam_height);

	cvSetMouseCallback(INPUT_WINDOW, mouseCallback, &userdata);

	setupWindows(&userdata);

	IplImage *input = userdata.input[0];
	IplImage *output = userdata.output[0];

	int use_cam = 1;
	int flip = 1;
	while ((userdata.key = cvWaitKey(userdata.timestep)) != 27) { // wait 50 ms (20 FPS) or for ESC key
		IplImage *cam = cvQueryFrame(cv_cap); // get frame
		if (!cam) {
			printf("no input\n");
			continue;
		}

		switch (userdata.key) {
		case ' ':
			use_cam = !use_cam;
			break;
		case 'F':
			flip = !flip;
			break;
		}

		if (!use_cam) {
			operateImage(&userdata);
		} else {
			cvCopy(cam, input, NULL);
			if (flip) {
				cvFlip(input, NULL, 1);
			}
			operateImage(&userdata);
		}
		//cvResizeWindow(INPUT_WINDOW, input->width, input->height);
		cvShowImage(INPUT_WINDOW, input);

		//cvResizeWindow(OUTPUT_WINDOW, output->width, output->height);
		cvShowImage(OUTPUT_WINDOW, output);
		cvResizeWindow(OUTPUT_WINDOW, cam_width / 2, cam_height / 2);
	}
	/* clean up */
	cvReleaseCapture(&cv_cap);

	freeSessionUserdata(&userdata);
	destroyWindows();

	cvDestroyWindow(INPUT_WINDOW);
	cvDestroyWindow(OUTPUT_WINDOW);

	return 0;
}
