#ifndef APOLLO_WAVE_CONTROL_H
#define APOLLO_WAVE_CONTROL_H

#include "CompoundControl.h"
#include "Buttons.h"

class Waveform;
class WaveDisplay;
class ImageButton;

class WaveControl;
class WavePoint;

class WavePointButton : public BaseButton
{
protected:
	WaveControl *waveParent = nullptr;
	WavePoint	*wavePoint = nullptr;
	Waveform	*waveform = nullptr;

	virtual void onDrag(APoint m_pos, AVec d_pos, bool direct) override;

public:
	WavePointButton(ParentElement *parent_, WavePoint *wave_point, Waveform *waveform_, GuiStateFlags s_flags);
	virtual ~WavePointButton();
	
	static const GuiPropFlags PROP_FLAGS;

	virtual void update(const Time &dt) override;
};



class WaveControl : public CompoundControl
{
private:
	Waveform		*wave		= nullptr;
	WaveDisplay		*waveDisp	= nullptr;
	ImageButton		*sin		= nullptr,
					*square		= nullptr,
					*sawtooth	= nullptr,
					*triangle	= nullptr;
	
	std::vector<WavePointButton*> buttons;

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