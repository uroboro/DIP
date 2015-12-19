#ifndef CLICK_H
#define CLICK_H

#include <opencv2/core/core_c.h>

typedef struct _click {
	CvPoint origin;
	int event;
	int flags;

	int down;
	int down_p;
	int down_edge;
	int up_edge;
} Click;

Click makeClick(CvPoint origin, int event, int flags);
int printClick(Click click);

int processClick(Click click);

typedef struct _drag {
	CvPoint start;
	CvPoint end;
} Drag;

int printDrag(Drag drag);

#endif /* CLICK_H */
