#ifndef APOLLO_WAVE_DISPLAY_H
#define APOLLO_WAVE_DISPLAY_H

#include "LineGraphDisplay.h"
#include "GraphData.h"

#include "Audio.h"

#include <vector>

class Waveform;


struct WaveData : public GraphData<s_time, FAudioSample>
{
	const FAudioSample *waveData;

	WaveData(const FAudioSample *wave_data, int n_points);
	virtual ~WaveData();
	virtual void update(const Time &dt) override;
	
	//virtual void drawData(Range<s_time> range, GlInterface &gl) const override;
	virtual void getData(FRange range, float x_step, std::vector<GPoint> &out_data) const override;
};

class WaveDisplay : public LineGraphDisplay<s_time, FAudioSample>
{
private:
	WaveData waveData;

public:
	WaveDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, Waveform *waveform);
	virtual ~WaveDisplay();
	
	static const GuiPropFlags PROP_FLAGS;

	void setData(const Waveform *waveform);
	const WaveData* getData() const;
	
	//Waveform* getWaveform();
	//void setWaveform(Waveform *waveform);

	//void drawData(GlInterface &gl);
	//virtual void draw(GlInterface &gl) override;
};


#endif	//APOLLO_WAVE_DISPLAY_H