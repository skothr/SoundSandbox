#include "MidiTrackControl.h"

#include "MidiDataDisplay.h"
#include "MidiBufferNode.h"

#include "Cursor.h"

/////MIDI TRACK CONTROL/////
const GuiPropFlags	MidiTrackControl::PROP_FLAGS = PFlags::HARD_BACK;
const AVec			MidiTrackControl::PADDING = AVec(5.0f, 5.0f);

MidiTrackControl::MidiTrackControl(ParentElement *parent_, APoint a_pos, float height, GuiStateFlags s_flags, MidiBufferNode *t_node, float seconds_per_pixel)
	: GuiElement(parent_, a_pos, AVec(100.0f, height), GuiProps(s_flags, PROP_FLAGS)),
		CompoundControl(GuiProps(s_flags, PROP_FLAGS)),
		node(t_node), secondsPerPixel(seconds_per_pixel)
{
	dataDisp = new MidiDataDisplay(this, APoint(PADDING), size - PADDING*2.0f, DEFAULT_STATE_FLOAT, node->getData(), GraphProps::FIXED_WIDTH, seconds_per_pixel);
	setSize(dataDisp->getSize() + PADDING*2.0f);
	
	setAllBgStateColors(Color(0.5f, 0.5f, 0.5f, 1.0f));
}

MidiTrackControl::~MidiTrackControl()
{
	if(dataDisp)
		delete dataDisp;
	dataDisp = nullptr;
}

void MidiTrackControl::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	if(direct && cursor && valid(b & MouseButton::LEFT))
	{
		cursor->setTimeSamples(max(m_pos.x - PADDING.x, 0)*secondsPerPixel);
	}
}

void MidiTrackControl::setCursor(Cursor *p_cursor)
{
	cursor = p_cursor;
}

void MidiTrackControl::setTrackNode(MidiBufferNode *new_node)
{
	node = new_node;
	dataDisp->setData(node->getData());		//TODO: Get seconds per pixel
	setSize(dataDisp->getSize() + PADDING*2.0f);
}

void MidiTrackControl::setSecondsPerPixel(float seconds_per_pixel)
{
	secondsPerPixel = seconds_per_pixel;

	dataDisp->setData(dataDisp->getData(), seconds_per_pixel);
	setSize(dataDisp->getSize() + PADDING*2.0f);
	parent->updateResources();
}

void MidiTrackControl::draw(GlInterface &gl)
{
	setSize(dataDisp->getSize() + PADDING*2.0f);
	dataDisp->setPos(APoint(PADDING));

	if(isolateViewport(gl))
	{
		drawBackground(gl);
		drawChildren(gl);
		
		if(cursor)
		{
			APoint a_cursor_pos = APoint(PADDING.x + cursor->getTimeRange().start/secondsPerPixel, 0.0f);
		
			gl.setColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
			gl.drawLine(APoint(a_cursor_pos.x, 0.0f), APoint(a_cursor_pos.x, size.y));
		}

		restoreViewport(gl);
	}
}