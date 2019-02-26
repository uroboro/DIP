#include <stdio.h>
#include <dirent.h>
#include <sys/syslimits.h>
#include "common.h"

#define CLAMP(X, MIN, MAX) \
({ \
	__typeof__ (X) _X = (X); \
	__typeof__ (MIN) _MIN = (MIN); \
	__typeof__ (MAX) _MAX = (MAX); \
	_X < _MIN ? _MIN : (_X > _MAX ? _MAX : _X); \
})

int main(int argc, char **argv, char **envp) {
	char basePath[] = "../Hands";

	struct dirent ** namelist;
	int n = scandir(basePath, &namelist, NULL, alphasort);
	if (n == -1) {
		printf("E: Could not open directory: '%s'\n", basePath);
		return 0;
	}

	size_t imgc = n - 2;
	char ** filePaths = (char **)calloc(imgc, sizeof(char *));
	for (size_t i = 0; i < imgc; i++) {
		filePaths[i] = (char *)calloc(PATH_MAX, sizeof(char *));
	}

	for (size_t i = 0; i < n; i++) {
		if (i > 1) {
			sprintf(filePaths[i-2], "%s/%s", basePath, namelist[i]->d_name);
		}
		free(namelist[i]);
	}
	free(namelist);

	FILE * fp = fopen("selection.txt", "a");
	IplImage * tmp3d = cvCreateImage(cvSize(400, 300), IPL_DEPTH_8U, 3);

	int32_t i = 0;
	while (1) {
		char * path = filePaths[i];
		IplImage * input = cvLoadImage(path, CV_LOAD_IMAGE_COLOR);
		if (!input) {
			printf("E: Couldn't open file: '%s'\n", path);
			continue;
		}

		cvResize(input, tmp3d, CV_INTER_LINEAR);
		CVSHOW(path, 70, 0, 400, 300, tmp3d);

		uint32_t key;
		switch (key = cvWaitKey(0)) {
			case 'y':
				fprintf(fp, "%s\n", path);
				break;
			case 0xf700: // up arrow
				// printf("^\n");
				break;
			case 0xf701: // down arrow
				// printf("v\n");
				break;
			case 0xf702: // left arrow
				// printf("<\n");
				i = CLAMP(i - 1, 0, imgc);
				break;
			case 0xf703: // right arrow
				// printf(">\n");
				i = CLAMP(i + 1, 0, imgc);
				break;
			default:
				printf("%x\n", key);
				break;
		}
		cvDestroyWindow(path);
		cvReleaseImage(&input);
	}
	cvReleaseImage(&tmp3d);
	fclose(fp);

	for (size_t i = 0; i < imgc; i++) {
		free(filePaths[i]);
	}
	free(filePaths);

	return 0;
}
