#ifndef CLICK_H
#define CLICK_H

#include <opencv2/core/core_c.h>

#include "common.h"

DIP_EXTERN typedef struct _click {
	CvPoint origin;
	int event;
	int flags;

	int down;
	int down_p;
	int down_edge;
	int up_edge;
} Click;

DIP_EXTERN Click makeClick(CvPoint origin, int event, int flags);
DIP_EXTERN int printClick(Click click);

DIP_EXTERN int processClick(Click click);

DIP_EXTERN typedef struct _drag {
	CvPoint start;
	CvPoint end;
} Drag;

DIP_EXTERN int printDrag(Drag drag);

#endif /* CLICK_H */
