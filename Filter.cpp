#include "Filter.h"

#include <math.h>

Modifier::Modifier()
{ }


ConstMod::ConstMod()
{ }

void ConstMod::modify(SampleState &state, const SampleInfo &info, double t)
{
	//Modify info

	state.frequency *= 0.25*sin(t*0.001);
	
	//double x = 16000*sin(t*0.0002);
	//if(x < 0) x = 0;

	//info.amplitude = x;

	//Cool Sounds!
	////1:
	//double x = 16000*sin(t*t*0.00002);
	//if(x < 0) x = 0;
	//info.amplitude = x;
}