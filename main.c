#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <getopt.h>

int main(int argc, char **argv, char **envp) {
	char *imageInPath = NULL;
	char *imageInPath2 = NULL;
	char *imageOutPath = "AoutPic.png";
	char *dylibPath = NULL;

	int opt;
	while ((opt = getopt(argc, argv, "d:f:o:F:")) != -1) {
		switch (opt) {

		case 'd':
			dylibPath = optarg;
			break;

		case 'f':
			imageInPath = optarg;
			break;

		case 'F':
			imageInPath2 = optarg;
			break;

		case 'o':
			imageOutPath = optarg;
			break;
		}
	}

	if (imageInPath == NULL || dylibPath == NULL) { printf("missing args\n"); return 3; }

	void *dylib = dlopen(dylibPath, RTLD_NOW);
	if (!dylib) { printf("!\"%s\" couldn't be loaded.\n", dylibPath); return 2; }

		int (*operar)(const char *, const char *, const char *);
		operar = (int (*)(const char *, const char *, const char *))dlsym(dylib, "operar");
		if (!operar) { printf("!\"operar\" couldn't be found.\n"); return 1; }
		return operar(imageInPath, imageInPath2, imageOutPath);
}

// vim:ft=objc
