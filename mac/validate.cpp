#include <stdio.h>
#include "common.h"

int main(int argc, char **argv, char **envp) {
	if (argc < 2) {
		return -1;
	}

	char * path = argv[1];
	IplImage * input = cvLoadImage(path, CV_LOAD_IMAGE_COLOR);
	if (!input) {
		printf("E: Couldn't open file: '%s'\n", path);
		return -1;
	}
	IplImage * tmp3d = cvCreateImage(cvSize(400, 300), IPL_DEPTH_8U, 3);

	cvResize(input, tmp3d, CV_INTER_LINEAR);
	CVSHOW(path, 70, 0, 400, 300, tmp3d);

	printf("0x%x\n", cvWaitKey(0));
	cvDestroyWindow(path);

	cvReleaseImage(&input);
	cvReleaseImage(&tmp3d);

	return 0;
}
