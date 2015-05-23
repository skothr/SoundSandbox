#ifndef APOLLO_MIDI_DATA_DISPLAY_H
#define APOLLO_MIDI_DATA_DISPLAY_H

#include "GraphDisplay.h"
#include "GraphData.h"
#include "MIDI.h"

class MidiData;
/*
struct MidiGraphData : public GraphData<double, MidiIndex>
{
	const MidiData		*mData;

	MidiGraphData(const MidiData *midi_data);
	virtual ~MidiGraphData();

	virtual void getData(FRange range, float x_step, std::vector<GPoint> &out_data) const override;
};
*/

class MidiDataDisplay : public GraphDisplay
{
protected:
protected:
	const MidiData		*midiData;

	virtual void onSizeChanged(AVec d_size) override;
	virtual void drawGraph(GlInterface &gl) override;

public:
	MidiDataDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, const MidiData *midi_data, GraphProps g_props, float seconds_per_pixel = -1.0f);
	virtual ~MidiDataDisplay();

	static const GuiPropFlags PROP_FLAGS;

	void setData(const MidiData *midi_data, float seconds_per_pixel = -1.0f);
	const MidiData* getData() const;

	/*
	const MidiData *midiData = nullptr;

	void updateData();

	virtual void onSizeChanged(AVec d_size) override;

	virtual void drawGraph(GlInterface &gl) override;

public:
	MidiDataDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, const MidiData *midi_data, GraphProps g_props);
	virtual ~MidiDataDisplay();

	static const GuiPropFlags PROP_FLAGS;

	void setData(const MidiData *midi_data);
	const MidiData* getData() const;
	*/

};

#endif	//APOLLO_MIDI_DATA_DISPLAY_H