#include "AudioTrackControl.h"

//#include "TrackNodes.h"
#include "AudioDataDisplay.h"
#include "AudioBufferNode.h"

#include "Cursor.h"

/////AUDIO TRACK CONTROL/////

const GuiPropFlags	AudioTrackControl::PROP_FLAGS	= PFlags::HARD_BACK;
const AVec			AudioTrackControl::PADDING		= AVec(5.0f, 5.0f);

AudioTrackControl::AudioTrackControl(ParentElement *parent_, APoint a_pos, float height, GuiStateFlags s_flags, AudioBufferNode *t_node, float samples_per_pixel)
	: GuiElement(parent_, a_pos, AVec(100.0f, height), GuiProps(s_flags, PROP_FLAGS)),
		CompoundControl(GuiProps(s_flags, PROP_FLAGS)),
		node(t_node), samplesPerPixel(samples_per_pixel)
{
	dataDisp = new AudioDataDisplay(this, APoint(PADDING), size - PADDING*2.0f, DEFAULT_STATE_FLOAT, node->getData(), GraphProps::FIXED_WIDTH, false, false, true, samples_per_pixel);
	setSize(dataDisp->getSize() + PADDING*2.0f);
	
	setAllBgStateColors(Color(0.5f, 0.5f, 0.5f, 1.0f));
}

AudioTrackControl::~AudioTrackControl()
{
	if(dataDisp)
		delete dataDisp;
	dataDisp = nullptr;
}

void AudioTrackControl::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	if(direct && cursor && valid(b & MouseButton::LEFT))
	{
		cursor->setTimeSamples(max(m_pos.x - PADDING.x, 0)*samplesPerPixel);
	}
}

void AudioTrackControl::setCursor(Cursor *p_cursor)
{
	cursor = p_cursor;
}


void AudioTrackControl::setTrackNode(AudioBufferNode *new_node)
{
	node = new_node;
	dataDisp->setData(node->getData(), false, false, true, samplesPerPixel);
	setSize(dataDisp->getSize() + PADDING*2.0f);
}

void AudioTrackControl::setSamplesPerPixel(float samples_per_pixel)
{
	samplesPerPixel = samples_per_pixel;

	dataDisp->setData(dataDisp->getData(), false, false, true, samples_per_pixel);
	setSize(dataDisp->getSize() + PADDING*2.0f);
	parent->updateResources();
}

void AudioTrackControl::draw(GlInterface &gl)
{
	setSize(dataDisp->getSize() + PADDING*2.0f);
	dataDisp->setPos(APoint(PADDING));

	if(isolateViewport(gl))
	{
		drawBackground(gl);
		drawChildren(gl);

		if(cursor)
		{
			APoint a_cursor_pos = APoint(PADDING.x + cursor->getSampleRange().start/samplesPerPixel, 0.0f);
		
			gl.setColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
			gl.drawLine(APoint(a_cursor_pos.x, 0.0f), APoint(a_cursor_pos.x, size.y));
		}

		restoreViewport(gl);
	}
}