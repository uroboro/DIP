#include <stdio.h>
#include "common.h"
#include "ocv_hand.h"

#define INPUT_WINDOW	"Input Window"
#define OUTPUT_WINDOW	"Output Window"

int main(int argc, char **argv, char **envp) {
	if (argc > 1) {
		// printf("Processing file: %s\n", argv[1]);
		IplImage * input = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
		IplImage * tmp3d = cvCreateImage(cvSize(400, 300), IPL_DEPTH_8U, 3);
		cvResize(input, tmp3d, CV_INTER_LINEAR);
		cvReleaseImage(&input);

		IplImage * output = cvCloneImage(tmp3d);
		cvSet(output, cvScalarAll(0), NULL);

		cvCvtColor(tmp3d, tmp3d, CV_BGR2RGB);
		ocv_handAnalysis(tmp3d, output);
		// cvCvtColor(tmp3d, tmp3d, CV_RGB2BGR);
		// cvCvtColor(output, output, CV_RGB2BGR);

		// CVSHOW(INPUT_WINDOW, 70, 0, 400, 30, tmp3d);
		// CVSHOW(OUTPUT_WINDOW, 80 + 400, 0, 400, 30, output);
		//
		// cvWaitKey(0);

		cvReleaseImage(&tmp3d);
		cvReleaseImage(&output);

		return 0;
	}

#define USE_IPLIMAGE 01
#if USE_IPLIMAGE
	CvCapture * cv_cap = cvCaptureFromCAM(CV_CAP_ANY);
	if (!cv_cap) {
		printf("E: Could not open camera.\n");
		cvWaitKey(0);
		return -1;
	}

	IplImage * input = cvQueryFrame(cv_cap);
	IplImage * tmp3d = cvCreateImage(cvSize(input->width/2, input->height/2), input->depth, 3);
	IplImage * output = cvCloneImage(tmp3d);
	cvSet(output, cvScalarAll(0), NULL);

	// cvReleaseImage(&tmp3d);

	int use_cam = 1;
	int flip = 1;
	int key = -1;
	while ((key = cvWaitKey(1)) != 27) { // wait 50 ms (20 FPS) or for ESC key
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

			if (flip) {
				cvFlip(input, NULL, 1);
			}
		}

		// IplImage *tmp3d = cvCreateImage(cvSize(input->width/2, input->height/2), input->depth, 3);
		cvResize(input, tmp3d, CV_INTER_LINEAR);

		cvCvtColor(input, input, CV_BGR2RGB);
		ocv_handAnalysis(tmp3d, output);
		cvCvtColor(input, input, CV_RGB2BGR);
		cvCvtColor(output, output, CV_RGB2BGR);

		CVSHOW(INPUT_WINDOW, 70, 0, tmp3d->width, tmp3d->height, tmp3d);
		CVSHOW(OUTPUT_WINDOW, 80 + output->width, 0, output->width, output->height, output);
	}
	cvReleaseImage(&tmp3d);

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
