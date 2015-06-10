#ifndef APOLLO_DATA_DISPLAY_H
#define APOLLO_DATA_DISPLAY_H

#include "CompoundControl.h"
#include "Audio.h"
#include "MIDI.h"
#include "Timing.h"
#include "NodeDataTypes.h"

class Cursor;

class AudioDisplay : public CompoundControl
{
private:
	static const AudioChunk *defaultChunk;

	const std::vector<DataChunk*>	*audioData = nullptr;

	bool						flipChunks = false,			//Whether to flip the order of the chunks within the data
								flipChunkSamples = false,	//Whether to flip to order of the samples within each chunk

								drawStatuses = false;

	Cursor						*cursor = nullptr;
	Time						cursorOffset = 0.0;		//Offset (seconds) from beginning of cursor time to beginning of this
														// display's time

	float						samplesPerPixel = 0.0f,
								cursorPos = 0.0f;		//Position (pixels) of the cursor in parent's virtual space.
		
protected:
	void onSizeChanged(AVec d_size) override;

public:
	AudioDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, bool draw_statuses);
	virtual ~AudioDisplay() = default;

	void setCursor(Cursor *p_cursor, double offset);

	void setData(const std::vector<DataChunk*> *audio_data, bool flip_chunks, bool flip_chunk_samples);
	const std::vector<DataChunk*>* getAudioData() const;

	c_time getNumChunks() const;
	
	virtual void onMouseDown(APoint m_pos, MouseButton b) override;
	
	//virtual void update(const Time &dt) override;

	void drawData(GlInterface &gl);
	void drawDataVel(GlInterface &gl);

	virtual void draw(GlInterface &gl) override;
};

class MidiDisplay : public CompoundControl
{
private:
	const MidiData *midiData = nullptr;

	Cursor *cursor = nullptr;

	//TODO: take in sample rate (if needed)
	const int TEMP_SAMPLERATE = 44100;

	float			secondsPerPixel = 0.0f,
					samplesPerPixel = 0.0f;
	
protected:
	void onSizeChanged(AVec d_size) override;

public:
	MidiDisplay(ParentElement *parent_, APoint a_pos, AVec a_size);
	virtual ~MidiDisplay() = default;

	void setCursor(Cursor *p_cursor);
	
	void setData(const MidiData *midi_data);
	const MidiData* getData() const;
	
	void drawData(GlInterface &gl);
	virtual void draw(GlInterface &gl) override;
};



#endif	//APOLLO_DATA_DISPLAY_H