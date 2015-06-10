#include "AudioDataDisplay.h"

#include <algorithm>

#include "AudioData.h"
#include "AudioChunk.h"

/////AUDIO GRAPH DATA/////

AudioGraphData::AudioGraphData(const AudioData *audio_data, bool flip_chunks, bool flip_chunk_samples, bool color_status)
	: GraphData<s_time, AudioSample>(audio_data ? audio_data->getNumChunks()*audio_data->getChunkSize() - 1 : 0),
		aData(audio_data), flipChunks(flip_chunks), flipChunkSamples(flip_chunk_samples), colorStatus(color_status)
{ }

AudioGraphData::~AudioGraphData()
{ }

void AudioGraphData::update(const Time &dt)
{

}

void AudioGraphData::getData(Range<float> range, float x_step, std::vector<GPoint> &data_out) const
{
	//FOR AUDIO SAMPLES

	c_time num_data_chunks = aData->getNumChunks();
	
	Range<s_time> s_range((s_time)floor(range.start), (s_time)ceil(range.end));
	Range<c_time> c_range((c_time)floor(floor(range.start) / (float)aData->getChunkSize()), (c_time)ceil(ceil(range.end) / (float)aData->getChunkSize()));

	int num_data = (int)ceil((float)s_range.length()/x_step);
	data_out.clear();
	data_out.reserve(num_data);

	//std::vector<AudioAmpChunk*> amp_data(c_range.length());
	//for(auto &a : amp_data)
	//	a = new AudioAmpChunk(aData->getChunkSize());
	//aData->getAmpData(amp_data, c_range);

	std::vector<float> amp_data;
	aData->sampleData(amp_data, range.start, x_step, num_data);

	for(int p = 0; p < num_data; p++)
	{
		
		float x = range.start + (float)p*x_step;

		/*
		s_time	prev_s = (s_time)floor(x) % AUDIO_CHUNK_SIZE,
				next_s = prev_s + 1;

		bool	wrap = (next_s >= AUDIO_CHUNK_SIZE);

		c_time	prev_c = (c_time)floor(x) / AUDIO_CHUNK_SIZE,
				next_c = prev_c + (wrap ? 1 : 0);
		
		float diff = abs(x - (float)(prev_s + prev_c*AUDIO_CHUNK_SIZE));
		
		next_s -= wrap ? AUDIO_CHUNK_SIZE : 0;
		
		next_c = (next_c >= num_data_chunks ? 0 : next_c);

		//Switch direction if applicable
		if(flipChunks)
		{
			prev_c = num_data_chunks - 1 - prev_c;
			next_c = num_data_chunks - 1 - next_c;
		}
		if(flipChunkSamples)
		{
			prev_s = AUDIO_CHUNK_SIZE - 1 - prev_s;
			next_s = AUDIO_CHUNK_SIZE - 1 - next_s;
		}

		float	prev_y = (float)(*amp_data[prev_c - c_range.start]->getData())[prev_s],
				next_y = (float)(*amp_data[next_c - c_range.start]->getData())[next_s],
				alpha = diff;
				*/

		//data_out.push_back(GPoint(x, lerp(prev_y, next_y, alpha)));
		data_out.push_back(GPoint(x, amp_data[p]));

	}
	
	//for(auto a : amp_data)
	//	if(a) delete a;
}

/*
void AudioGraphData::drawData(Range<s_time> range, GlInterface &gl) const
{
	c_time	low_c = range.start / AUDIO_CHUNK_SIZE,
			high_c = range.end / AUDIO_CHUNK_SIZE;
	s_time	low_s = range.start % AUDIO_CHUNK_SIZE,
			high_s = range.end % AUDIO_CHUNK_SIZE,
			
			s_index = 0;

	//data_out.resize(range.length());

	
	c_time	start_c = (flipChunks ? aData->size() - 1 - low_c : low_c),
			end_c = (flipChunks ? aData->size() - 1 - high_c : high_c),
			step_c = (flipChunks ? -1 : 1);

	s_time	start_s_offset = (flipChunks ? high_s : low_s),
			end_s_offset = (flipChunks ? low_s : high_s),

			start_s = (flipChunkSamples ? AUDIO_CHUNK_SIZE - 1 : 0),
			end_s = (flipChunkSamples ? 0 : AUDIO_CHUNK_SIZE - 1),

			step_s = (flipChunkSamples ? -1 : 1);

	start_s_offset = (flipChunkSamples ? AUDIO_CHUNK_SIZE - 1 - start_s_offset : start_s_offset);
	end_s_offset = (flipChunkSamples ? AUDIO_CHUNK_SIZE - 1 - end_s_offset : end_s_offset);


	
	gl.setColor(Color(0.0f, 0.0f, 1.0f, 1.0f));

	std::vector<TVertex> vertices;
	vertices.reserve(y_data.size());
	vertices.push_back(TVertex(prev_point, Color(0.0f, 0.0f, 1.0f, 1.0f)));

	bool first_c = true;

	for(c_time c = start_c; (flipChunks ? c >= end_c : c < end_c); c += step_c)
	{
		const AudioSample *c_samples = (*aData)[c]->getData();

		s_time	s = (first_c ? start_s_offset : start_s),
				ls = (c == end_c ? end_s_offset : end_s);

		for(; (flipChunkSamples ? s > ls : s < ls); s += step_s)
		{
			//data_out[s_index] = c_samples[s];
			s_index++;
		}
	}
}
*/


/////AUDIO VEL GRAPH DATA/////
/*
AudioVelGraphData::AudioVelGraphData(const AudioVelData *audio_data, bool flip_chunks, bool flip_chunk_samples)
	: GraphData<s_time, AudioVelSample>(audio_data ? audio_data->getNumChunks()*audio_data->getChunkSize() - 1 : 0),
		aData(audio_data), flipChunks(flip_chunks), flipChunkSamples(flip_chunk_samples)
{ }

AudioVelGraphData::~AudioVelGraphData()
{ }


void AudioVelGraphData::getData(Range<float> range, float x_step, std::vector<GPoint> &data_out) const
{
	//FOR AUDIO VEL SAMPLES
	
	Range<s_time> s_range((s_time)floor(range.start), (s_time)ceil(range.end));
	Range<c_time> c_range((c_time)floor(floor(range.start) / (float)aData->getChunkSize()), (c_time)ceil(ceil(range.end) / (float)aData->getChunkSize()));

	int num_data = (int)ceil((float)s_range.length()/x_step);
	data_out.clear();
	data_out.reserve(num_data);
	
	//c_time curr_c = s_range.start / AUDIO_CHUNK_SIZE;
	//AudioChunkData chunk_data;
	//(*aData)[curr_c]->getAudioData(chunk_data);
	
	std::vector<AudioAmpChunk*> amp_data(c_range.length());
	for(auto &a : amp_data)
		a = new AudioAmpChunk(aData->getChunkSize());
	aData->getAmpData(amp_data, c_range);

	for(int p = 0; p < num_data; p++)
	{
		float x = range.start + (float)p*x_step;

		s_time	prev_s = (s_time)floor(x) % AUDIO_CHUNK_SIZE,
				next_s = prev_s + 1;

		bool	wrap = (next_s >= AUDIO_CHUNK_SIZE);

		c_time	prev_c = (c_time)floor(x) / AUDIO_CHUNK_SIZE,
				next_c = prev_c + (wrap ? 1 : 0);
		
		float diff = abs(x - (float)(prev_s + prev_c*AUDIO_CHUNK_SIZE));
		
		next_s -= wrap ? AUDIO_CHUNK_SIZE : 0;
		
		next_c = (next_c >= aData->getNumChunks() ? 0 : next_c);

		//Switch direction if applicable
		if(flipChunks)
		{
			prev_c = aData->getNumChunks() - 1 - prev_c;
			next_c = aData->getNumChunks() - 1 - next_c;
		}
		if(flipChunkSamples)
		{
			prev_s = AUDIO_CHUNK_SIZE - 1 - prev_s;
			next_s = AUDIO_CHUNK_SIZE - 1 - next_s;
		}

		//if(prev_c != curr_c)
		//{
		//	curr_c = prev_c;
		//	(*aData)[curr_c]->getAudioData(chunk_data);
		//}

		float	prev_y = (float)(*amp_data[prev_c - c_range.start]->getData())[prev_s];//chunk_data[prev_s];

		//if(wrap || next_c != curr_c)
		//{
		//	curr_c = next_c;
		//	(*aData)[curr_c]->getAudioData(chunk_data);
		//}

		float	next_y = (float)(*amp_data[next_c - c_range.start]->getData())[next_s];//chunk_data[next_s];

		data_out.push_back(GPoint(x, lerp(prev_y, next_y, diff)));

	}
	
	for(auto a : amp_data)
		if(a) delete a;
}
*/



/////AUDIO DATA DISPLAY/////
const GuiPropFlags AudioDataDisplay::PROP_FLAGS = PFlags::HARD_BACK;

AudioDataDisplay::AudioDataDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, const AudioData *audio_data, GraphProps g_props, bool flip_chunks, bool flip_chunk_samples, bool color_status, float samples_per_pixel)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		LineGraphDisplay<s_time, AudioVelSample>(nullptr, GVec((float)AUDIO_CHUNK_SIZE, 4000.0f), g_props),
		audioData(audio_data, flip_chunks, flip_chunk_samples, color_status)
{
	LineGraphDisplay<s_time, AudioVelSample>::setData(&audioData);
	setData(audio_data, flip_chunks, flip_chunk_samples, color_status, samples_per_pixel);
}

AudioDataDisplay::~AudioDataDisplay()
{ }

void AudioDataDisplay::setData(const AudioData *audio_data, bool flip_chunks, bool flip_chunk_samples, bool color_status, float samples_per_pixel)
{
	audioData.aData = audio_data;
	audioData.flipChunks = flip_chunks;
	audioData.flipChunkSamples = flip_chunk_samples;
	audioData.colorStatus = color_status;

	samples_per_pixel = (samples_per_pixel < 0.0f ? 20.0f : samples_per_pixel);

	if(audioData.aData)
	{
		audioData.size = audio_data->getNumChunks()*audio_data->getChunkSize() - 1;
		
		GPoint	min_p(0.0f, -AUDIO_MAX_AMPLITUDE),
				max_p((float)audioData.size, AUDIO_MAX_AMPLITUDE);

		RVec abs_to_graph(samples_per_pixel, (float)(max_p.y - min_p.y)/size.y);

		setGraph(min_p, max_p, abs_to_graph, standardUnitSize, gProps);
	}
	else
		audioData.size = 0;

	if(gProps & GProps::FIXED_WIDTH)
		setWidth((float)audioData.size * samples_per_pixel);
}

const AudioData* AudioDataDisplay::getData() const
{
	return audioData.aData;
}

void AudioDataDisplay::drawGraph(GlInterface &gl)
{
	LineGraphDisplay<s_time, AudioSample>::drawGraph(gl);
	
	if(audioData.colorStatus)
	{
		GPoint	v_origin = getVisibleOrigin(gl);
		GVec	v_size = getVisibleSize(gl);
				//v_svec = absoluteToGraphVec(gl.viewToAbsoluteVec(gl.getCurrView().size), false);
		AVec	gl_size = gl.getCurrView().size;

		s_time chunk_size = audioData.aData->getChunkSize();

		ChunkRange c_r((c_time)floor(v_origin.x/(float)chunk_size), (c_time)ceil((v_origin.x + v_size.x)/(float)chunk_size));

		const AudioData		*a_data = audioData.aData;
		const AudioVelData	*av_data = dynamic_cast<const AudioVelData*>(a_data);
		const AudioAmpData	*aa_data = dynamic_cast<const AudioAmpData*>(a_data);

		bool vel = (av_data != nullptr && aa_data == nullptr);

		for(c_time c = c_r.start; c < c_r.end; c++)
		{
			const AudioChunk *this_chunk = vel ? dynamic_cast<const AudioChunk*>((*av_data)[c])
												: dynamic_cast<const AudioChunk*>((*aa_data)[c]);

			if(this_chunk->getStatus() == DataStatus::CLEAN)
				gl.setColor(Color(0.0f, 1.0f, 0.0f, 0.2f));
			else
				gl.setColor(Color(1.0f, 0.0f, 0.0f, 0.2f));

			GPoint p(c*chunk_size + (gProps & GProps::FLIP_X ? 1 : 0), (gProps & GProps::FLIP_Y ? minData.y : maxData.y));
			GVec s(chunk_size*(gProps & GProps::FLIP_X ? -1 : 1), (maxData.y - minData.y)*(gProps & GProps::FLIP_X ? -1 : 1));

			Rect r(graphToAbsolutePoint(p), graphToAbsoluteVec(s));

			gl.drawRect(r);
		}

		//std::cout << "AUDIO: " << c_r.start << " -- " << c_r.end << "\n";
	}

}


/////AUDIO VEL DATA DISPLAY/////
/*
const GuiPropFlags AudioVelDataDisplay::PROP_FLAGS = PFlags::HARD_BACK;

AudioVelDataDisplay::AudioVelDataDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, const AudioVelData *audio_data, GraphProps g_props, bool flip_chunks, bool flip_chunk_samples)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		LineGraphDisplay<s_time, AudioVelSample>(nullptr, GVec((float)AUDIO_CHUNK_SIZE, 3200), g_props),
		audioData(audio_data, flip_chunks, flip_chunk_samples)
{
	setData(audio_data, flip_chunks, flip_chunk_samples);
}

AudioVelDataDisplay::~AudioVelDataDisplay()
{ }

void AudioVelDataDisplay::setData(const AudioVelData *audio_data, bool flip_chunks, bool flip_chunk_samples)
{
	LineGraphDisplay<s_time, AudioVelSample>::setData(&audioData);
	audioData.aData = audio_data;
	if(audioData.aData)
	{
		audioData.size = audio_data->getNumChunks()*audio_data->getChunkSize() - 1;
		audioData.flipChunks = flip_chunks;
		audioData.flipChunkSamples = flip_chunk_samples;

		if(audioData.size > 0)
		{
			std::vector<GPoint> data;
			audioData.getData(FRange(0.0, (float)audioData.size), 1.0f, data);

			//float		min_y = *std::min_element(data.begin(), data.end()),
			//			max_y = *std::max_element(data.begin(), data.end());

			setGraph(GPoint(0.0f, -AUDIO_MAX_AMPLITUDE), GPoint((float)audioData.size, AUDIO_MAX_AMPLITUDE), absoluteToGraphUnits, standardUnitSize, gProps);
		}
	}
	else
	{

	}
}

const AudioVelData* AudioVelDataDisplay::getData() const
{
	return audioData.aData;
}
*/