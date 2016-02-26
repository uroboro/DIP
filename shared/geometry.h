#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <opencv2/core/core_c.h>

// r*e^(j*f) -> (x, y)
#define cvPointPolar(r, f)		cvPointFrom32f(cvPoint2D32f((r) * cos((f)), (r) * sin((f))))

// s * (x, y)
#define cvPointScale(p, s)		cvPoint((s) * (p).x, (s) * (p).y)

// (x, y) -> r*e^(j*f)
#define cvPointModule(p)		sqrt(cvPointDot(p, p))
#define cvPointPhase(p) 		atan2((p).y, (p).x)

// p + q
#define cvPointAdd(p, q)		cvPoint((p).x + (q).x, (p).y + (q).y)
// p . q
#define cvPointDot(p, q)		((p).x * (q).x + (p).y * (q).y)
// p - q
#define cvPointSubtract(p, q)	cvPointAdd(p, cvPointScale(q, -1))

#define cvPointDistance(p, q)	cvPointModule(cvPointSubtract(p, q))
// (p + q) / 2
#define cvPointMidPoint(p, q)	cvPointScale(cvPointAdd(p, q), 0.5)

#define cvPointAngle(p, q)		acos(cvPointDot(p, q) / (cvPointModule(p) * cvPointModule(q)))

#define cvPointProject(p, q)	(cvPointDot(p, q) / cvPointModule(q)))

#define cvPointFromSize(p)		cvPoint((p).width, (p).height)

#define cvSizeScale(p, s)		cvSize((s) * (p).width, (s) * (p).height)

#endif /* GEOMETRY_H */
