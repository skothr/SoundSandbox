#ifndef APOLLO_MOD_DATA_H
#define APOLLO_MOD_DATA_H

#include <vector>

#include "Timing.h"
#include "Interpolation.h"

struct ModLevelPoint
{
	Time			t = 0.0;
	double				value = 0.0;

	InterpolationType	interp = Interp::LINEAR;	//Interpolation on this point's upper side
															//  (forward in time)

	ModLevelPoint(Time time, double mod_value, InterpolationType forward_interp)
		: t(time), value(mod_value), interp(forward_interp)
	{ }
};


class ModData
{
protected:
	std::vector<ModLevelPoint> data;

public:
	ModData();
	virtual ~ModData();

	std::vector<ModLevelPoint>* getData();
	const std::vector<ModLevelPoint>* getConstData() const;

	void addPoint(const ModLevelPoint &new_point);

	//Returns the data point that is closest to the given time
	ModLevelPoint* operator[](Time t);
	const ModLevelPoint* operator[](Time t) const;
};


#endif	//APOLLO_MOD_DATA_H