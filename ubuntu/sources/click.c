#include "click.h"
#include <stdio.h>
#include <opencv2/highgui/highgui_c.h>

//#pragma mark - click functions

Click makeClick(CvPoint origin, int event, int flags) {
	Click click;
	memset(&click, 0, sizeof(Click));
	click.origin = origin;
	click.event = event;
	click.flags = flags;

	click.down = -1;

	return click;
}

int printClick(Click click) {
	return printf("<Click origin={%d, %d} event=%d flags=%d down=%d(%d) edges=%d%d>", click.origin.x, click.origin.y, click.event, click.flags, click.down, click.down_p, click.down_edge, click.up_edge);
}

int processClick(Click click) {
	switch (click.event) {
	case -1:
		return -1;
	case CV_EVENT_LBUTTONDOWN:
	case CV_EVENT_RBUTTONDOWN:
	case CV_EVENT_MBUTTONDOWN:
		click.down = 1;
		break;
	case CV_EVENT_LBUTTONUP:
	case CV_EVENT_RBUTTONUP:
	case CV_EVENT_MBUTTONUP:
		click.down = 0;
		break;
	default:
		break;
	}

	if (click.down != click.down_p && click.down == 0) {
		click.down_edge = 1;
	}
	else {
		click.down_edge = 0;
	}
	if (click.down != click.down_p && click.down == 1) {
		click.up_edge = 1;
	}
	else {
		click.up_edge = 0;
	}

	click.down_p = click.down;

	return 0;
}

//#pragma mark - drag functions

int printDrag(Drag drag) {
	return printf("<Drag start={%d, %d} end={%d, %d}>", drag.start.x, drag.start.y, drag.end.x, drag.end.y);
}


