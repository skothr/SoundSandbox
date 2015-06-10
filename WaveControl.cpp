#include "WaveControl.h"

#include "Waveform.h"
#include "WaveDisplay.h"
#include "Buttons.h"
#include <functional>




/////WAVE POINT BUTTON/////
const GuiPropFlags WavePointButton::PROP_FLAGS = PFlags::HARD_BACK;

WavePointButton::WavePointButton(ParentElement *parent_, WavePoint *wave_point, Waveform *waveform_, GuiStateFlags s_flags)
	: GuiElement(parent_, APoint(wave_point->x*parent_->getSize().x, wave_point->y*parent_->getSize().y), AVec(5, 5), GuiProps(s_flags, PROP_FLAGS)),
		BaseButton(GuiProps(s_flags, PROP_FLAGS), ""),
		waveParent((WaveControl*)parent_), wavePoint(wave_point), waveform(waveform_)
{

}

WavePointButton::~WavePointButton()
{

}

void WavePointButton::onDrag(APoint m_pos, AVec d_pos, bool direct)
{
	if(direct)
	{
		setPos(pos + d_pos);
		wavePoint->x = pos.x;
		wavePoint->y = pos.y;

		waveform->cacheWaveform();
	}
}

void WavePointButton::update(const Time &dt)
{
	setPos(APoint(wavePoint->x*waveParent->getSize().x, wavePoint->y*waveParent->getSize().y));
}



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
	
	/*
	std::vector<WavePoint> *points = waveform->getPoints();
	buttons.clear();
	buttons.reserve(points->size());

	for(auto &p : *points)
	{
		buttons.push_back(new WavePointButton(this, &p, waveform, DEFAULT_STATE));
	}
	*/

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

	std::vector<WavePoint> *points = waveform->getPoints();

	//buttons.clear();
	//buttons.reserve(points->size());

	//for(auto &p : *points)
	//	buttons.push_back(new WavePointButton(this, &p, waveform, DEFAULT_STATE));

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
