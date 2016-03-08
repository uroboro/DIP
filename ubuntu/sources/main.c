#include <stdio.h>
#include "common.h"
#include "ocv_hand.h"

#define INPUT_WINDOW	"Input Window"
#define OUTPUT_WINDOW	"Output Window"

IplImage *ocv_handSpriteCreate(char *path) {
	char buf[256]; sprintf(buf, "Resources/%s", path);
	return cvLoadImage(buf, CV_LOAD_IMAGE_COLOR);
}

int main(int argc, char *argv[], char *envp[]) {
	CvCapture *cv_cap = cvCaptureFromCAM(CV_CAP_ANY);
	if (!cv_cap) {
		printf("E: Could not open camera.\n");
		cvWaitKey(0);
		return -1;
	}

	IplImage *input = cvQueryFrame(cv_cap);
	IplImage *output = cvCloneImage(input);

	int use_cam = 1;
	int flip = 1;
	int key = -1;
	while ((key = cvWaitKey(50)) != 27) { // wait 50 ms (20 FPS) or for ESC key
		switch (key) {
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

			cvCvtColor(cam, input, CV_BGR2RGB);
			if (flip) {
				cvFlip(input, NULL, 1);
			}
		}

		ocv_handAnalysis(input, output);

		cvCvtColor(input, input, CV_RGB2BGR);
		CVSHOW(INPUT_WINDOW, 0, 0, input->width/2, input->height/2, input);

		cvCvtColor(output, output, CV_RGB2BGR);
		CVSHOW(OUTPUT_WINDOW, 0, output->height*2/3, output->width/2, output->height/2, output);
	}

	/* clean up */
	cvReleaseCapture(&cv_cap);

	cvReleaseImage(&output);

	cvDestroyWindow(INPUT_WINDOW);
	cvDestroyWindow(OUTPUT_WINDOW);

	return 0;
}
