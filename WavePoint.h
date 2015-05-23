#ifndef APOLLO_WAVE_POINT_H
#define APOLLO_WAVE_POINT_H

#include "Interpolation.h"

struct WavePoint
{
	double	x = 0,
			y = 0;

	InterpolationType interp_type = Interp::LINEAR;

	WavePoint()
	{ }
	WavePoint(double x_, double y_, InterpolationType interp_type_)
		: x(x_), y(y_), interp_type(interp_type_)
	{ }

	Point2d getPoint() const
	{ return Point2d(x, y); }
};


#endif	//APOLLO_WAVE_POINT_H