#include "MidiDataDisplay.h"

#include "MIDI.h"


/////MIDI GRAPH DATA/////
/*
MidiGraphData::MidiGraphData(const MidiData *midi_data)
	: GraphData<double, MidiIndex>(midi_data ? midi_data->getSpan().length() : 0.0f),
		mData(midi_data)
{

}

void MidiGraphData::getData(FRange range, float x_step, std::vector<GPoint> &out_data) const
{

}
*/


/////MIDI DATA DISPLAY/////
const GuiPropFlags MidiDataDisplay::PROP_FLAGS = PFlags::HARD_BACK;

MidiDataDisplay::MidiDataDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, const MidiData *midi_data, GraphProps g_props, float seconds_per_pixel)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		GraphDisplay(GPoint(0.0f, 0.0f), GPoint(0.0f, 0.0f), RVec(1.0f, 1.0f), GVec(1.0f, 1.0f), g_props),
		midiData(midi_data)
{
	setData(midi_data, seconds_per_pixel);
}

MidiDataDisplay::~MidiDataDisplay()
{ }

void MidiDataDisplay::setData(const MidiData *midi_data, float seconds_per_pixel)
{
	midiData = midi_data;
	
	seconds_per_pixel = (seconds_per_pixel < 0.0f ? 0.005f : seconds_per_pixel);

	if(midiData)
	{
		FRange x_range((float)0.0f, (float)midiData->getSpan().end);

		RVec abs_to_graph(seconds_per_pixel, (float)(NUM_MIDI_NOTES - 1)/size.y);

		setGraph(GPoint(0.0f, 0.0f), GPoint(x_range.end, (float)(NUM_MIDI_NOTES - 1)), abs_to_graph, GVec(1.5f, 5.0f), gProps);
	}
	
	if(gProps & GProps::FIXED_WIDTH)
		setWidth((float)midiData->getSpan().end * seconds_per_pixel);
}

const MidiData* MidiDataDisplay::getData() const
{
	return midiData;
}

void MidiDataDisplay::onSizeChanged(AVec d_size)
{
	GraphDisplay::onSizeChanged(d_size);
}

void MidiDataDisplay::drawGraph(GlInterface &gl)
{
	GPoint	v_origin = getVisibleOrigin(gl);
	GVec	v_size = getVisibleSize(gl);
	AVec	gl_size = gl.getCurrView().size;

	TimeRange t_range(v_origin.x, v_origin.x + v_size.x);

	gl.setColor(Color(0.0f, 0.3f, 0.1f, 1.0f));
	for(auto m : midiData->getConstNotes(t_range))
	{
		gl.drawLine(graphToAbsolutePoint(GPoint(m->range.start, m->index)), graphToAbsolutePoint(GPoint(m->range.end, m->index)));
	}
}