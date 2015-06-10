#ifndef APOLLO_WAVEFORM_H
#define APOLLO_WAVEFORM_H

#include <vector>
#include <stdint.h>

#include "Control.h"

#include "Audio.h"
#include "Filter.h"
#include "Sampling.h"
#include "Timing.h"

#include "Interpolation.h"
#include "WavePoint.h"

//#include "Saveable.h"

class ModTrack;


inline double lerp(double s_x, WavePoint p1, WavePoint p2)
{
	return lerp(s_x, p1.getPoint(), p2.getPoint());
}

inline double cosInterp(double s_x, WavePoint p1, WavePoint p2)
{
	return cosInterp(s_x, p1.getPoint(), p2.getPoint());
}

inline double smoothInterp(double s_x, WavePoint p1, WavePoint p2)
{
	return smoothInterp(s_x, p1.getPoint(), p2.getPoint());
}
inline double smootherInterp(double s_x, WavePoint p1, WavePoint p2)
{
	return smootherInterp(s_x, p1.getPoint(), p2.getPoint());
}

inline double cubeInterp(double s_x, WavePoint p0, WavePoint p1, WavePoint p2, WavePoint p3)
{
	return cubeInterp(s_x, p0.getPoint(), p1.getPoint(), p2.getPoint(), p3.getPoint());
}



class Waveform// : public Saveable
{
public:
	static const int NUM_CACHE_POINTS = 8192;

private:
	std::vector<WavePoint> points;
	FAudioSample cache[NUM_CACHE_POINTS];
	
	void cacheBetween(int p1_index, int p2_index);

public:
	Waveform();
	//Waveform(const WaveformDesc &wave_desc);

	void cacheWaveform();

	void addPoint(WavePoint p);
	void removePoint(int index);
	void clearPoints();

	std::vector<WavePoint>* getPoints();
	const FAudioSample* getCache() const;

	void setSin();
	void setSawtooth();
	void setSquare();
	void setTriangle();

	//Samples the wave with the given state --> returns a value between 0.0 and 1.0
	FAudioSample sample(const SampleState &state) const;
	FAudioVelSample sampleVel(const SampleState &state) const;
	/*
	void sampleChunk(SampleState &state, const SampleInfo &info, AudioChunk **data, c_time num_chunks);

	void sampleChunk(s_time start, SampleInfo &info, int num_samples, AudioSample *p_data, const std::vector<Modifier*> &mods);
	void sampleChunkAdd(s_time start, SampleInfo &info, int num_samples, AudioSample *p_data, const std::vector<Modifier*> &mods);
	*/
	
protected:
	//virtual void updateDesc() override;
};

#endif	//APOLLO_WAVEFORM_H