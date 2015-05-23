#include "WaveControl.h"

#include "Waveform.h"
#include "WaveDisplay.h"
#include "Buttons.h"
#include <functional>


/////WAVE CONTROL/////
const GuiPropFlags WaveControl::PROP_FLAGS = PFlags::HARD_BACK;

const AVec WaveControl::PADDING = 5.0f;

WaveControl::WaveControl(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, Waveform *waveform)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		CompoundControl(GuiProps(s_flags, PROP_FLAGS)),
		wave(waveform)
{
	setAllBgStateColors(Color(0.4f, 0.4f, 0.4f, 1.0f));

	waveDisp = new WaveDisplay(this, PADDING, a_size - PADDING*2.0f, DEFAULT_STATE_FLOAT, wave);

	sin = new ImageButton(this, APoint(5, 5), DEFAULT_STATE_FLOAT, "SIN");
	square = new ImageButton(this, APoint(sin->getPos().x + sin->getSize().x + 5, 5), DEFAULT_STATE_FLOAT, "SQUARE");
	sawtooth = new ImageButton(this, APoint(square->getPos().x + square->getSize().x + 5, 5), DEFAULT_STATE_FLOAT, "SAWTOOTH");
	triangle = new ImageButton(this, APoint(sawtooth->getPos().x + sawtooth->getSize().x + 5, 5), DEFAULT_STATE_FLOAT, "TRIANGLE");

	waveDisp->attachTo(this, AttachSide::LEFT, 5);
	waveDisp->attachTo(this, AttachSide::RIGHT, 5);
	waveDisp->attachTo(sin, AttachSide::TOP, 5);
	waveDisp->attachTo(this, AttachSide::BOTTOM, 5);

	//std::vector<GuiElement*> children;
	//children.push_back(waveDisp);
	//children.push_back(sin);
	//children.push_back(square);
	//children.push_back(sawtooth);
	//children.push_back(triangle);

	//CompoundControl::init(nullptr, &children);

	//sin->resolveAttachments();
	resolveAttachments();
}

WaveControl::~WaveControl()
{
	if(waveDisp) delete waveDisp;
	waveDisp = nullptr;

	if(sin) delete sin;
	sin = nullptr;
	if(square) delete square;
	square = nullptr;
	if(sawtooth) delete sawtooth;
	sawtooth = nullptr;
	if(triangle) delete triangle;
	triangle = nullptr;
}


const Waveform* WaveControl::getWaveform() const
{
	return wave;
}

void WaveControl::setWaveform(Waveform *waveform)
{
	wave = waveform;

	sin->setClickFunction(std::bind(&Waveform::setSin, wave));
	square->setClickFunction(std::bind(&Waveform::setSquare, wave));
	sawtooth->setClickFunction(std::bind(&Waveform::setSawtooth, wave));
	triangle->setClickFunction(std::bind(&Waveform::setTriangle, wave));

	waveDisp->setData(wave);
}

/*
void WaveControl::draw(GlInterface &gl)
{
	if(isolateViewport(gl))
	{
		drawBackground(gl);
		drawChildren(gl);

		restoreViewport(gl);
	}
}
*/
