#include "Samplers.h"


MidiSampler::MidiSampler()
	: waveform(nullptr)
{ }

MidiSampler::MidiSampler(Waveform *wave)
	: waveform(wave)
{
	//for(int i = 0; i < 1000000; i++)
	//{
		//notes.push_back(MidiNote(69 + i, 100, i*5000, (i + 1)*5000));
		//notes.push_back(MidiNote(69 + i + 12, 20, i*5000 + 2500, (i + 1)*5000 + 2500));
		//notes.push_back(MidiNote(69 + i + 4, 20, i*5000, (i + 1)*5000));
		//notes.push_back(MidiNote(69 + i + 6, 20, i*5000, (i + 1)*5000));
		//notes.push_back(MidiNote(69 + i + 8, 20, i*5000, (i + 1)*5000));
	//}

	notes.push_back(MidiNote(69, 100, 0, 1000000));
}

void MidiSampler::setWaveform(Waveform *wave)
{
	waveform = wave;
}

void MidiSampler::getChunk(AudioSample *p_data, s_time data_start, int num_samples, int sample_rate, const std::vector<Modifier*> &mods)
{
	SampleInfo info;
	info.sample_rate = sample_rate;

	s_time cursor = data_start;
	s_time last_cursor = -1;
	int data_offset = 0;

	for(s_time t = 0; t < sample_rate; t++)
		p_data[t] = 0;

	s_time data_end = data_start + num_samples;

	for(unsigned int i = 0; i < notes.size(); i++)
	{
		if(notes[i].end >= data_start && notes[i].start <= data_end)
		{
			//TODO: Calculate phase using distance from note.start and start
			info.phase = 0;

			info.frequency = getFrequency(notes[i].index);
			info.amplitude = 160.0*notes[i].velocity;

			s_time chunk_start = notes[i].start > data_start ? notes[i].start : data_start,
				chunk_end = notes[i].end < data_end ? notes[i].end : data_end;

			int samples = chunk_end - chunk_start;

			waveform->sampleChunkAdd(chunk_start, info, samples, p_data + notes[i].start - data_start, mods);
		}
	}
}