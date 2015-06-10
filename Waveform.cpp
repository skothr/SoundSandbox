#include "Waveform.h"

#include "Container.h"
#include "Sampling.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif	//_USE_MATH_DEFINES
#include <math.h>

#include <iostream>

//void fftTest();


////WAVEFORM DEFINITIONS

Waveform::Waveform()
{
	//clearPoints();
	setSin();
	//cacheWaveform();
	//fftTest();
}
/*
Waveform::Waveform(const WaveformDesc &wave_desc)
{
	points.reserve(wave_desc.points.size());
	points.insert(points.end(), wave_desc.points.begin(), wave_desc.points.end());
	cacheWaveform();
}
*/
void Waveform::cacheWaveform()
{
	cacheBetween(0, points.size() - 1);
}

void Waveform::cacheBetween(int p1_index, int p2_index)
{
	//return if indices are out of bounds or equal
	if(p1_index < 0 || p1_index >= points.size() || p2_index < 0 || p2_index >= points.size() || p1_index == p2_index)
	{
		std::cout << "WARNING: waveform caching between indices " << p1_index << " and " << p2_index << "failed.\n";
		return;
	}

	int lowIndex = (p1_index < p2_index) ? p1_index : p2_index,
		highIndex = (p1_index < p2_index) ? p2_index : p1_index;

	int lowCacheIndex = points[p1_index].x*(NUM_CACHE_POINTS - 1),
		highCacheIndex = points[p2_index].x*(NUM_CACHE_POINTS - 1);
	
	int curr_pointIndex = lowIndex - 1;
	bool start = true;
	
	WavePoint prev_point, this_point, next_point, next_point2;

	for(int i = lowCacheIndex; i <= highCacheIndex; i++)
	{
		float x = (float)i/(NUM_CACHE_POINTS - 1);

		if(x >= points[curr_pointIndex + 1].x || start)
		{
			start = false;
			curr_pointIndex++;

			//Previous point
			if(curr_pointIndex > 0)
				prev_point = points[curr_pointIndex - 1];
			else
				prev_point = points[points.size() - 1];

			//This point
			this_point = points[curr_pointIndex];

			//Next point
			if(curr_pointIndex + 1 < points.size())
				next_point = points[curr_pointIndex + 1];
			else
				next_point = points[0];

			//Next next point
			if(curr_pointIndex + 2 < points.size())
				next_point2 = points[curr_pointIndex + 2];
			else
				next_point2 = points[1];
			
			//Don't interpolate between points that are on top of each other, just use the next value
			//Otherwise, it'll be infinity
			if(this_point.x == next_point.x)
			{
				if(curr_pointIndex == 0)
					cache[i] = this_point.y;
				else
					cache[i] = next_point.y;
				continue;
			}
		}

		//Get alpha value for interpolation
		//double alpha = (x - this_point.x)/(next_point.x - this_point.x);

		//Interpolation, based off point interpolation type
		switch(this_point.interp_type)
		{
		case Interp::LINEAR:
			cache[i] = lerp(x, this_point, next_point);
			break;
		case Interp::CUBIC:
			cache[i] = cubeInterp(x, prev_point, this_point, next_point, next_point2);
			break;
		case Interp::SMOOTH:
			cache[i] = smoothInterp(x, this_point, next_point);
			break;
		case Interp::SMOOTHER:
			cache[i] = smootherInterp(x, this_point, next_point);
			break;
		case Interp::MATCH_ADJACENT:
			cache[i] = lerp(x, this_point, next_point);
			break;
		default:
			//Default to linear
			cache[i] = lerp(x, this_point, next_point);
			break;
		}
		

		//if(abs(i - NUM_CACHE_POINTS/2.0f) < 5)
		//	std::cout << cache[i] << "\n";
	}
}

void Waveform::addPoint(WavePoint p)
{
	//Keep within bounds
	if(p.x > 1.0) p.x = 1.0;
	if(p.x < 0.0) p.x = 0.0;
	if(p.y > 1.0) p.y = 1.0;
	if(p.y < -1.0) p.y = -1.0;

	for(int i = 0; i < points.size(); i++)
	{
		int index = -1;
		if(points[i].x == p.x)
		{
			//Add point after one that's already there
			if(i != points.size() - 1)
				index = i + 1;
			//Unless last point
			else
				index = i;
		}
		else if(p.x < points[i].x && i != 0)
		{
			//Add before unless first point
			index = i;
		}
		else if(i != points.size() - 1 && p.x < points[i + 1].x)
		{
			//Add after unless last point
			index = i + 1;
		}

		//Insert point if done
		if(index != -1)
		{
			points.insert(points.begin() + index, p);
			return; //index;
		}
	}

	points.push_back(p);

	//cacheWaveform();
}

void Waveform::removePoint(int index)
{
	points.erase(points.begin() + index);
}

void Waveform::clearPoints()
{
	points.clear();
	//Add default anchor points
	addPoint(WavePoint(0.0, 0.0, Interp::MATCH_ADJACENT));
	addPoint(WavePoint(1.0, 0.0, Interp::MATCH_ADJACENT));
}

std::vector<WavePoint>* Waveform::getPoints()
{
	return &points;
}

const FAudioSample* Waveform::getCache() const
{
	return cache;
}

#define NUM_SIN_POINTS 1024

void Waveform::setSin()
{
	std::cout << "WAVEFORM SIN\n";
	clearPoints();

	for(int i = 1; i < NUM_SIN_POINTS - 1; i++)
	{
		double x = (double)i/(NUM_SIN_POINTS - 1);
		addPoint(WavePoint(x, sin(2.0*M_PI*x), Interp::LINEAR));//Interp::COSINE));//Interp::CUBIC));
	}

	cacheWaveform();
}

void Waveform::setSawtooth()
{
	std::cout << "WAVEFORM SAWTOOTH\n";

	clearPoints();

	addPoint(WavePoint(0.5f, 1.0f, Interp::LINEAR));
	addPoint(WavePoint(0.5f, -1.0f, Interp::LINEAR));

	cacheWaveform();
}

void Waveform::setSquare()
{
	std::cout << "WAVEFORM SQUARE\n";

	clearPoints();

	addPoint(WavePoint(0.0f, 1.0f, Interp::LINEAR));
	addPoint(WavePoint(0.5f, 1.0f, Interp::LINEAR));
	addPoint(WavePoint(0.5f, -1.0f, Interp::LINEAR));
	addPoint(WavePoint(1.0f, -1.0f, Interp::LINEAR));

	cacheWaveform();
}

void Waveform::setTriangle()
{
	std::cout << "WAVEFORM TRIANGLE\n";

	clearPoints();

	addPoint(WavePoint(0.25f, 1.0f, Interp::LINEAR));
	addPoint(WavePoint(0.75f, -1.0f, Interp::LINEAR));

	cacheWaveform();
}

FAudioSample Waveform::sample(const SampleState &state) const
{
	return state.amplitude * cache[(unsigned int)(state.phase*(double)NUM_CACHE_POINTS)];
}

FAudioVelSample Waveform::sampleVel(const SampleState &state) const
{
	return ((AudioVelSample)(state.amplitude*cache[(unsigned int)(state.phase*(double)NUM_CACHE_POINTS)])
				- (AudioVelSample)(state.amplitude*cache[(unsigned int)(state.lastPhase*(double)NUM_CACHE_POINTS)]));
}

/*
void Waveform::updateDesc()
{
	objDesc = (objDesc ? objDesc : (ObjDesc*)(new WaveformDesc()));
	WaveformDesc *desc = dynamic_cast<WaveformDesc*>(objDesc);
	
	desc->points.clear();
	desc->points.reserve(points.size());
	desc->points.insert(desc->points.end(), points.begin(), points.end());
}
*/



/*
void Waveform::sampleChunk(SampleState &state, const SampleInfo &info, AudioChunk **data, c_time num_chunks)
{
	double phase_step = state.frequency/info.getSampleRate();

	for(c_time c = 0; c < num_chunks; c++)
	{
		s_time num_samples = data[c]->size;

		for(s_time s = 0; s < num_samples; s++)
		{
			data[c]->data[s] = (AudioSample)(cache[(unsigned int)(state.phase*NUM_CACHE_POINTS)]*state.amplitude);
			info.stepBy(phase_step);
		}
	}
}


void Waveform::sampleChunk(s_time start, SampleInfo &info, int num_samples, AudioSample *p_data, const std::vector<Modifier*> &mods)
{
	double phase_step = info.frequency/info.getSampleRate();

	for(int i = 0; i < num_samples; i++)
	{
		for(unsigned int j = 0; j < mods.size(); j++)
		{
			mods[j]->modify(info, start);
		}
		phase_step = info.frequency/info.getSampleRate();

		int x = (int)(info.phase*NUM_CACHE_POINTS) % NUM_CACHE_POINTS;
		p_data[i] = (AudioSample)(cache[x]*info.amplitude);
		info.stepBy(phase_step);

		start++;
	}
}

void Waveform::sampleChunkAdd(s_time start, SampleInfo &info, int num_samples, AudioSample *p_data, const std::vector<Modifier*> &mods)
{
	double phase_step = info.frequency/info.getSampleRate();

	for(int i = 0; i < num_samples; i++)
	{
		for(unsigned int j = 0; j < mods.size(); j++)
		{
			mods[j]->modify(info, start);
		}
		phase_step = info.frequency/info.getSampleRate();

		int x = (int)(info.phase*NUM_CACHE_POINTS) % NUM_CACHE_POINTS;
		p_data[i] += (AudioSample)(cache[x]*info.amplitude);
		info.stepBy(phase_step);

		start++;
	}
}
*/
