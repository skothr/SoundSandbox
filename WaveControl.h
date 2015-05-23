#ifndef APOLLO_WAVE_CONTROL_H
#define APOLLO_WAVE_CONTROL_H

#include "CompoundControl.h"

class Waveform;
class WaveDisplay;
class ImageButton;

class WaveControl : public CompoundControl
{
private:
	Waveform		*wave		= nullptr;
	WaveDisplay		*waveDisp	= nullptr;
	ImageButton		*sin		= nullptr,
					*square		= nullptr,
					*sawtooth	= nullptr,
					*triangle	= nullptr;

public:
	WaveControl(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, Waveform *waveform);
	virtual ~WaveControl();

	static const GuiPropFlags PROP_FLAGS;

	const Waveform* getWaveform() const;
	void setWaveform(Waveform *waveform);

	static const AVec PADDING;

	//virtual void draw(GlInterface &gl) override;
};

/*
class WaveDisplay : public CompoundControl
{
private:
	Waveform		*wave;

public:
	WaveDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, Waveform *waveform);
	virtual ~WaveDisplay() = default;
	
	Waveform* getWaveform();
	void setWaveform(Waveform *waveform);

	void drawData(GlInterface &gl);
	virtual void draw(GlInterface &gl) override;
};
*/

#endif	//APOLLO_WAVE_CONTROL_H