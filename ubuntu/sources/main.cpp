#include <stdio.h>
#include "common.h"
#include "ocv_hand.h"

#define INPUT_WINDOW	"Input Window"
#define OUTPUT_WINDOW	"Output Window"

DIP_EXTERN IplImage *ocv_handSpriteCreate(char *path);
IplImage *ocv_handSpriteCreate(char *path) {
	char buf[256]; sprintf(buf, "Resources/%s", path);
	return cvLoadImage(buf, CV_LOAD_IMAGE_COLOR);
}

int main(int argc, char *argv[], char *envp[]) {
#define USE_IPLIMAGE 0
#if USE_IPLIMAGE
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
#else
	cv::VideoCapture cap(0);
	if (!cap.isOpened()) {
		printf("E: Could not open camera.\n");
		cvWaitKey(0);
		return -1;
	}

	cv::Mat input, output;

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
			cv::Mat cam;
			cap >> cam; // get frame
			if (cam.empty()) {
				printf("W: No input.\n");
				continue;
			}

			cv::cvtColor(cam, input, CV_BGR2RGB);
			if (flip) {
				cv::flip(input, input, 1);
			}
		}

		//ocv_handAnalysisMat(input, output);
		input.copyTo(output);

		cv::cvtColor(input, input, CV_RGB2BGR);
		imshow(INPUT_WINDOW, input);//CVSHOW(INPUT_WINDOW, 0, 0, input->width/2, input->height/2, input);

		cv::cvtColor(output, output, CV_RGB2BGR);
		imshow(OUTPUT_WINDOW, output);//CVSHOW(OUTPUT_WINDOW, 0, output->height*2/3, output->width/2, output->height/2, output);
	}
	//cvDestroyWindow(INPUT_WINDOW);
	//cvDestroyWindow(OUTPUT_WINDOW);
#endif

	return 0;
}
