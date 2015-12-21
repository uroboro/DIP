// Win32_Console.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "operateImage.h"

int main(int argc, char *argv[], char *envp[]) {
#if 01
	//char name[] = "resources/Edges.png";
	//char name[] = "resources/Untitled100x100.png";
	//char name[] = "resources/Untitled20x20.png";
	char name[] = "resources/mask.png";
	IplImage *aaa = cvLoadImage(name, CV_LOAD_IMAGE_GRAYSCALE);
	IplImage *tmp3d = cvCreateImage(cvGetSize(aaa), IPL_DEPTH_8U, 3);
	filterByVolume(aaa, tmp3d, 5000);
	cvWaitKey(0);
	cvReleaseImage(&aaa); cvReleaseImage(&tmp3d);
	return 0;
#endif
#if 0
	char name[] = "/home/uroboro/Downloads/floating_palace_by_jonone.jpg";
	IplImage *aaa = cvLoadImage(name, -1);
	if (!aaa) { printf("no image\n"); return 1; }
	cvNamedWindow(name, 0); cvResizeWindow(name, aaa->width, aaa->height); cvShowImage(name, aaa);
	cvWaitKey(0); cvReleaseImage(&aaa); cvDestroyWindow(name);
	return 0;
#endif

	CvCapture *cv_cap = cvCaptureFromCAM(CV_CAP_ANY);
	if (!cv_cap) {
		printf("E: Could not open camera.\n");
		cvWaitKey(0);
		return -1;
	}

	int cam_width = (int)cvGetCaptureProperty(cv_cap, CV_CAP_PROP_FRAME_WIDTH);
	int cam_height = (int)cvGetCaptureProperty(cv_cap, CV_CAP_PROP_FRAME_HEIGHT);
	CvSize cam_size = cvSize(cam_width, cam_height);

	Userdata userdata = getSessionUserdata(cam_size);

	cvNamedWindow(INPUT_WINDOW, 0);
	cvNamedWindow(OUTPUT_WINDOW, 0);
	cvMoveWindow(OUTPUT_WINDOW, 0, 300);

	cvSetMouseCallback(INPUT_WINDOW, mouseCallback, &userdata);

	setupWindows(&userdata);

	IplImage *input = userdata.input[0];

	int use_cam = 1;
	int flip = 1;
	while ((userdata.key = cvWaitKey(userdata.timestep)) != 27) { // wait 50 ms (20 FPS) or for ESC key
		switch (userdata.key) {
		case ' ':
			use_cam = !use_cam;
			break;
		case 'F':
			flip = !flip;
			break;
		}

		if (use_cam) {
			IplImage *cam = cvQueryFrame(cv_cap); // get frame
			if (!cam) {
				printf("W: No input.\n");
				continue;
			}

			cvCopy(cam, input, NULL);
			if (flip) {
				cvFlip(input, NULL, 1);
			}
		}
		operateImage(&userdata);

		cvResizeWindow(INPUT_WINDOW, cam_width / 2, cam_height / 2);
		cvShowImage(INPUT_WINDOW, input);

		cvResizeWindow(OUTPUT_WINDOW, cam_width / 2, cam_height / 2);
		cvShowImage(OUTPUT_WINDOW, userdata.output[0]);
	}

	/* clean up */
	cvReleaseCapture(&cv_cap);

	freeSessionUserdata(&userdata);
	destroyWindows(&userdata);

	cvDestroyWindow(INPUT_WINDOW);
	cvDestroyWindow(OUTPUT_WINDOW);

	return 0;
}
