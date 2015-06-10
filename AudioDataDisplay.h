#ifndef APOLLO_AUDIO_DATA_DISPLAY_H
#define APOLLO_AUDIO_DATA_DISPLAY_H

#include "LineGraphDisplay.h"
#include "GraphData.h"

#include "Audio.h"
#include "Range.h"

#include <vector>

class AudioData;
class AudioAmpChunk;

struct AudioGraphData : public GraphData<s_time, AudioSample>
{
	const AudioData				*aData;
	bool						flipChunks,
								flipChunkSamples,
								colorStatus;

	std::vector<AudioAmpChunk*> amp_data;

	AudioGraphData(const AudioData *audio_data, bool flip_chunks, bool flip_chunk_samples, bool color_status);
	virtual ~AudioGraphData();

	virtual void update(const Time &dt) override;

	virtual void getData(FRange range, float x_step, std::vector<GPoint> &out_data) const override;
	//virtual void drawData(Range<s_time> range, GlInterface &gl) const override;
};
/*
struct AudioVelGraphData : public GraphData<s_time, AudioVelSample>
{
	const AudioVelData	*aData;
	bool				flipChunks,
						flipChunkSamples;

	AudioVelGraphData(const AudioVelData *audio_data, bool flip_chunks, bool flip_chunk_samples);
	virtual ~AudioVelGraphData();

	virtual void getData(FRange range, float x_step, std::vector<GPoint> &out_data) const override;
	//virtual void drawData(Range<s_time> range, GlInterface &gl) const override;
};
*/
class AudioDataDisplay : public LineGraphDisplay<s_time, AudioSample>
{
protected:
	AudioGraphData		audioData;

	virtual void drawGraph(GlInterface &gl) override;

public:
	AudioDataDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, const AudioData *audio_data, GraphProps g_props, bool flip_chunks = false, bool flip_chunk_samples = false, bool color_status = true, float samples_per_pixel = -1.0f);
	virtual ~AudioDataDisplay();

	static const GuiPropFlags PROP_FLAGS;

	void setData(const AudioData *audio_data, bool flip_chunks, bool flip_chunk_samples, bool color_status, float samples_per_pixel = -1.0f);
	const AudioData* getData() const;
};

/*
class AudioVelDataDisplay : public LineGraphDisplay<s_time, AudioVelSample>
{
protected:
	AudioVelGraphData		audioData;

public:
	AudioVelDataDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, const AudioVelData *audio_data, GraphProps g_props, bool flip_chunks = false, bool flip_chunk_samples = false);
	virtual ~AudioVelDataDisplay();

	static const GuiPropFlags PROP_FLAGS;

	void setData(const AudioVelData *audio_data, bool flip_chunks, bool flip_chunk_samples);
	const AudioVelData* getData() const;
};
*/
#endif	//APOLLO_AUDIO_DATA_DISPLAY_H