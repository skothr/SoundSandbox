#include "DataDisplay.h"

#include "Node.h"
#include "Cursor.h"

#define DRAW_CHUNK_LINES true

/////AUDIO DISPLAY/////


AudioDisplay::AudioDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, bool draw_statuses)
	: CompoundControl(parent_, a_pos, a_size, true, false),
		drawStatuses(draw_statuses)
{
	setAllBackgroundColors(Color(0.5f, 0.5f, 0.5f, 1.0f));
	
	onSizeChanged(AVec());
}

void AudioDisplay::onSizeChanged(AVec d_size)
{
	if(audioData)
	{
		samplesPerPixel = (float)audioData->size()*(float)AUDIO_CHUNK_SIZE/size.x;
	}
}

void AudioDisplay::setCursor(Cursor *p_cursor, double offset)
{
	cursor = p_cursor;
	cursorOffset = offset;
}

void AudioDisplay::setData(const std::vector<DataChunk*> *audio_data, bool flip_chunks, bool flip_chunk_samples)
{
	audioData = (const std::vector<DataChunk*>*)audio_data;

	flipChunks = flip_chunks;
	flipChunkSamples = flip_chunk_samples;

	onSizeChanged(AVec());
}


const std::vector<DataChunk*>* AudioDisplay::getAudioData() const
{
	return audioData;
}

/*
const StatusList* AudioDisplay::getStatusData() const
{
	return statusData;
}
*/

void AudioDisplay::onMouseDown(APoint m_pos, MouseButton b)
{
	if(valid(b & MB::LEFT) && cursor)
	{
		cursor->setTimeSamples(m_pos.x*samplesPerPixel);
	}
}

void AudioDisplay::drawData(GlInterface &gl)
{
	//(Up here for now to minimize flickering)
	s_time cursor_time = (cursor ? cursor->getSampleRange().start : 0);
	c_time num_data_chunks = audioData->size();

	//Get view origin/size in absolute space
	APoint	av_pos = gl.viewToAbsolutePoint(APoint(0, 0));
	AVec	v_size = gl.getCurrView().size,
			av_size = gl.viewToAbsoluteVec(v_size);

	const float
				//Basic multipliers
				sample_mult = 1.0f/AUDIO_MAX_AMPLITUDE,	//Multiplier to transform an AudioSample to a 0.0 to 1.0 scale
				chunk_mult = 1.0f/AUDIO_CHUNK_SIZE,		//Multiplier to convert samples to chunks
				a_to_v = gl.absoluteToViewVec(AVec(1.0, 0.0f)).x,	//Converts absolute sizes to view sizes
				v_to_a = 1.0f/a_to_v,
			
				//Samples per physical pixel in absolute and view spaces
				//TODO: Figure out whether this should be in absolute or view space
				a_spp = samplesPerPixel,
				a_pps = 1.0f/a_spp,
				v_spp = gl.absoluteToViewVec(AVec(samplesPerPixel, 0)).x,
				v_pps = a_to_v*a_pps,//1.0f/v_spp,

				//Chunks per pixel
				a_cpp = a_spp*chunk_mult,
				a_ppc = (float)AUDIO_CHUNK_SIZE*a_pps,
				v_cpp = v_spp*chunk_mult,
				v_ppc = (float)AUDIO_CHUNK_SIZE*v_pps;
			

	//Left-most point, centered vertically (VIRTUAL SPACE)
	APoint origin(0, size.y/2.0f);

	//Sample values
	s_time	sample_offset = (s_time)(av_pos.x*a_spp), 
			num_samples = (s_time)ceil(av_size.x*a_spp),
			first_chunk_so = sample_offset % AUDIO_CHUNK_SIZE,
			last_chunk_so = AUDIO_CHUNK_SIZE - (first_chunk_so + num_samples) % AUDIO_CHUNK_SIZE,
			total_samples = first_chunk_so + num_samples + last_chunk_so;

	//Chunk values
	c_time	start_chunk = (c_time)(sample_offset*chunk_mult),
			num_chunks = (c_time)ceil(total_samples*chunk_mult),
			last_chunk = start_chunk + num_chunks;

	//Check for errors
	if (sample_offset < 0 || start_chunk >= num_data_chunks || num_chunks <= 0)
	{
		std::cout << "ERROR --> \n";

		if(sample_offset < 0)
			std::cout << "\t\tSAMPLE_OFFSET LESS THAN 0\n";
		if (start_chunk >= num_data_chunks)
			std::cout << "\t\tSTART_CHUNK_TOO_BIG\n";
		if(num_chunks <= 0)
			std::cout << "\t\tNUM_CHUNKS <= 0\n";

		std::cout << "\tSAMPLE_OFFSET:  " << sample_offset << "\n";
		std::cout << "\tSTART_CHUNK:    " << start_chunk << "\n";
		std::cout << "\tNUM CHUNKS:     " << num_chunks << "\n";

		return;
	}
	
	if(start_chunk + num_chunks > num_data_chunks)
		num_chunks = num_data_chunks - start_chunk;
	
	//Get chunk sizes to sample (one sample per on-screen pixel)
	float	desired_chunk_size = v_pps*(float)AUDIO_CHUNK_SIZE;
	s_time	actual_chunk_size = (s_time)ceil(desired_chunk_size);
	float	actual_to_desired = desired_chunk_size/(float)actual_chunk_size;		//Converts sampled chunk to desired chunk
	

	const AudioChunk** pData = (const AudioChunk**)((*audioData).data() + start_chunk);
	
	//Number of pixels up until start_chunk
	float start_px = start_chunk*a_ppc;

	//Allocate space for sampled chunks
	AudioChunk	**s_data = new AudioChunk*[num_chunks];
	for(c_time i = 0; i < num_chunks; i++)
		s_data[i] = new AudioChunk(actual_chunk_size);

	//Sample chunks
	AStatus status = sampleChunks(pData, s_data, num_chunks, SampleMethod::AVERAGE, false);

	if(!statusGood(status))
	{
		std::cout << "AUDIO DISPLAY -->\n" << status << "\n";
		return;
	}

	////DRAW////
	std::vector<TVertex> points;
	Color col;

	//Draw status highlights
	if(drawStatuses)
	{
		for(c_time c = 0; c < num_chunks; c++)
		{
			//Set status color
			switch(pData[c]->getStatus())
			{
			case DataStatus::CLEAN:
				gl.setColor(0.5f, 0.65f, 0.55f);
				break;
			case DataStatus::DIRTY:
				gl.setColor(0.65f, 0.5f, 0.5f);
				break;
			default:
				//Unknown status
				gl.setColor(1.0f, 0.0f, 1.0f);
			}

			//Draw current chain
			gl.drawRect(APoint((start_chunk + c)*a_ppc, 0.0f), AVec(a_ppc, size.y));
		}

		/*
		DataStatus curr_status = pData[0]->getStatus();
		ChunkRange c_r(-1, -1);

		for(c_time c = 0; c < num_chunks; c++)
		{
			c_time d_c = start_chunk + c;
			DataStatus d_status = pData[c]->getStatus();
			bool continue_chain = (d_status == curr_status);

			if(continue_chain || c + 1 == num_chunks)
			{
				//Continue chain
				c_r.start = (c_r.start < 0 ? d_c : c_r.start);
				c_r.end = d_c + 1;
			}

			if(!continue_chain || c + 1 == num_chunks)
			{
				//Set status color
				switch(curr_status)//aData[d_c]->status)
				{
				case DataStatus::CLEAN:
					gl.setColor(0.5f, 0.65f, 0.55f);
					break;
				case DataStatus::DIRTY:
					gl.setColor(0.65f, 0.5f, 0.5f);
					break;
				default:
					//Unknown status
					gl.setColor(1.0f, 0.0f, 1.0f);
				}

				//Draw current chain
				gl.drawRect(APoint(c_r.start*a_ppc, 0.0f), AVec(c_r.length()*a_ppc, size.y));

				//Start new chain
				c_r.start = d_c;
				c_r.end = d_c + 1;
				curr_status = d_status;
			}
		}
		*/
	}
	
	//Draw center line
	gl.setColor(0.2f, 0.2f, 0.2f, 1.0f);
	gl.drawLine(APoint(start_px, origin.y), APoint(start_px + total_samples*a_pps, origin.y));
	
	//TEST: Draw every data point (to compare approximation with)
	if(Keyboard::keyDown(Keys::K_Q))
	{
		points.clear();
		points.reserve(num_samples);
		col = Color(1.0f, 0.0f, 0.0f, 1.0f);

		const c_time	c_start = (flipChunks ? (num_chunks - 1) : 0),
						c_step = (flipChunks ? -1 : 1);
		const s_time	s_start = (flipChunkSamples ? (AUDIO_CHUNK_SIZE - 1) : 0),
						s_step = (flipChunkSamples ? -1 : 1);

		for(c_time c = c_start; (!flipChunks && c < num_chunks) || (flipChunks && c >= 0); c += c_step)
		{
			const AudioSample *c_data = pData[c]->getData();

			for(s_time s = s_start; (!flipChunkSamples && s < AUDIO_CHUNK_SIZE) || (flipChunkSamples && s >= 0); s += s_step)
			{
				//Calculate number of samples from the start (adjusting for flipped data or chunks)
				s_time s_x = (flipChunks ? (num_chunks - 1 - c) : c)*AUDIO_CHUNK_SIZE
								+ (flipChunkSamples ? (AUDIO_CHUNK_SIZE - 1 - s) : s);

				APoint p(start_px + s_x*a_pps, origin.y - origin.y*(float)c_data[s]*sample_mult);
				points.push_back(TVertex(p, col));
			}
		}
		gl.drawShape(GL_LINE_STRIP, points);
	}


	//Draw lines connecting samples (with approximate, sampled data)
	points.clear();
	points.reserve(num_chunks*desired_chunk_size);
	col = Color(0.0f, 0.0f, 0.0f, 1.0f);

	const c_time	c_start = (flipChunks ? (num_chunks - 1) : 0),
					c_step = (flipChunks ? -1 : 1);
	const s_time	s_start = (flipChunkSamples ? (actual_chunk_size - 1) : 0),
					s_step = (flipChunkSamples ? -1 : 1);

	for(c_time c = c_start; (!flipChunks && c < num_chunks) || (flipChunks && c >= 0); c += c_step)
	{
		const AudioSample *c_data = s_data[c]->getData();

		for(s_time s = s_start; (!flipChunkSamples && s < actual_chunk_size) || (flipChunkSamples && s >= 0); s += s_step)
		{
			const s_time s_x = (flipChunks ? (num_chunks - 1 - c) : c)*actual_chunk_size
								+ (flipChunkSamples ? (actual_chunk_size - 1 - s) : s);

			APoint p(start_px + s_x*actual_to_desired*v_to_a, origin.y - origin.y*(float)c_data[s]*sample_mult);
			points.push_back(TVertex(p, col));
		}
	}
	gl.drawShape(GL_LINE_STRIP, points);

	/*
	glBegin(GL_LINE_STRIP);
		glColor3f(0.0f, 0.0f, 0.0f);
		
		s_time sample = 0;
		for(c_time c = 0; c < num_chunks; c++)
		{
			std::vector<AudioSample>	&c_data = s_data[c]->data;

			for(s_time s = first_chunk_po*(c == 0); s < actual_chunk_size && sample < num_samples; s++, sample++)
			{
				GlPoint gl_pos = toGlPoint(APoint((float)sample*chunk_ratio, origin.y*(1.0f - (float)c_data[s]*sample_mult)), vv_size);
				
				glVertex2f(gl_pos.x, gl_pos.y);
			}
		}
	glEnd();
	*/

	/*
	//Draw lines connecting samples (as squares)
	glBegin(GL_QUADS);
		glColor3f(0.0f, 0.0f, 0.0f);
		
		s_time sample = 0;
		for(c_time c = 0; c < num_chunks; c++)
		{
			std::vector<AudioSample>	&c_data_max = s_max[c]->data,
										&c_data_min = s_min[c]->data;

			for(s_time s = first_chunk_po*(c == 0); s < actual_chunk_size && sample < num_pixels; s++, sample++)
			{
				float p_x = (float)sample*chunk_ratio;

				GlPoint gl_pos_max = toGlPoint(APoint(p_x - 0.5f, origin.y*(1.0f - (float)c_data_max[s]*sample_mult)), vv_size),
						gl_pos_min = toGlPoint(APoint(p_x + 0.5f, origin.y*(1.0f - (float)c_data_min[s]*sample_mult)), vv_size);
				
				//Draw vertical bar from max to min -- 1 pixel wide
				glVertex2f(gl_pos_max.x, gl_pos_max.y);
				glVertex2f(gl_pos_max.x, gl_pos_min.y);
				glVertex2f(gl_pos_min.x, gl_pos_max.y);
				glVertex2f(gl_pos_min.x, gl_pos_min.y);

			}
		}
	glEnd();
	*/
	
	if(DRAW_CHUNK_LINES)
	{
		points.clear();
		points.reserve(2*num_chunks);
		col = Color(0.4f, 0.4f, 0.9f, 0.3f);

		float x = av_pos.x + (AUDIO_CHUNK_SIZE - first_chunk_so)*a_pps;
		
		for(c_time c = 0; c < num_chunks; c++, x += a_ppc)
		{
			if(pData[c]->NOTE_ON)
			{
				gl.setColor(Color(1.0f, 0.0f, 0.0f, 0.5f));
				gl.drawRect(APoint(x - AUDIO_CHUNK_SIZE*a_pps, 0.0f), AVec(AUDIO_CHUNK_SIZE*a_pps, size.y));
			}
			
			col = Color(0.4f, 0.4f, 0.9f, 0.3f);

			points.push_back(TVertex(APoint(x, 0.0f), col));
			points.push_back(TVertex(APoint(x, size.y), col));
		}

		gl.drawShape(GL_LINES, points);
		points.clear();
	}
	

	//Draw cursor
	if(cursor)
	{
		float x = cursor_time*a_pps;
		
		gl.setColor(0.8f, 0.8f, 0.1f);
		gl.drawLine(APoint(x, 0.0f), APoint(x, size.y));
	}
	
	//Release sampled data
	if(s_data)
	{
		for(int i = 0; i < num_chunks; i++)
			delete s_data[i];
		delete[] s_data;
	}

	/*
	if(s_max)
	{
		for(int i = 0; i < num_chunks; i++)
			delete s_max[i];
		delete[] s_max;
	}
	if(s_min)
	{
		for(int i = 0; i < num_chunks; i++)
			delete s_min[i];
		delete[] s_min;
	}
	*/
}

void AudioDisplay::drawDataVel(GlInterface &gl)
{
	//(Up here for now to minimize flickering)
	s_time cursor_time = (cursor ? cursor->getSampleRange().start : 0);
	c_time num_data_chunks = audioData->size();

	//Get view origin/size in absolute space
	APoint	av_pos = gl.viewToAbsolutePoint(APoint(0, 0));
	AVec	v_size = gl.getCurrView().size,
			av_size = gl.viewToAbsoluteVec(v_size);

	const float
				//Basic multipliers
				sample_mult = 1.0f/AUDIO_MAX_AMPLITUDE,	//Multiplier to transform an AudioSample to a 0.0 to 1.0 scale
				chunk_mult = 1.0f/AUDIO_CHUNK_SIZE,		//Multiplier to convert samples to chunks
				a_to_v = gl.absoluteToViewVec(AVec(1.0, 0.0f)).x,	//Converts absolute sizes to view sizes
				v_to_a = 1.0f/a_to_v,
			
				//Samples per physical pixel in absolute and view spaces
				//TODO: Figure out whether this should be in absolute or view space
				a_spp = samplesPerPixel,
				a_pps = 1.0f/a_spp,
				v_spp = gl.absoluteToViewVec(AVec(samplesPerPixel, 0.0f)).x,
				v_pps = a_to_v*a_pps,//1.0f/v_spp,

				//Chunks per pixel
				a_cpp = a_spp*chunk_mult,
				a_ppc = (float)AUDIO_CHUNK_SIZE*a_pps,
				v_cpp = v_spp*chunk_mult,
				v_ppc = (float)AUDIO_CHUNK_SIZE*v_pps;
			

	//Left-most point, centered vertically (VIRTUAL SPACE)
	APoint origin(0, size.y/2.0f);

	//Sample values
	s_time	sample_offset = (s_time)(av_pos.x*a_spp), 
			num_samples = (s_time)ceil(av_size.x*a_spp),
			first_chunk_so = sample_offset % AUDIO_CHUNK_SIZE,
			last_chunk_so = AUDIO_CHUNK_SIZE - (first_chunk_so + num_samples) % AUDIO_CHUNK_SIZE,
			total_samples = first_chunk_so + num_samples + last_chunk_so;

	//Chunk values
	c_time	start_chunk = (c_time)(sample_offset*chunk_mult),
			num_chunks = (c_time)ceil(total_samples*chunk_mult),
			last_chunk = start_chunk + num_chunks;

	//Check for errors
	if (sample_offset < 0 || start_chunk >= num_data_chunks || num_chunks <= 0)
	{
		std::cout << "ERROR --> \n";

		if(sample_offset < 0)
			std::cout << "\t\tSAMPLE_OFFSET LESS THAN 0\n";
		if (start_chunk >= num_data_chunks)
			std::cout << "\t\tSTART_CHUNK_TOO_BIG\n";
		if(num_chunks <= 0)
			std::cout << "\t\tNUM_CHUNKS <= 0\n";

		std::cout << "\tSAMPLE_OFFSET:  " << sample_offset << "\n";
		std::cout << "\tSTART_CHUNK:    " << start_chunk << "\n";
		std::cout << "\tNUM CHUNKS:     " << num_chunks << "\n";

		return;
	}
	
	if(start_chunk + num_chunks > num_data_chunks)
		num_chunks = num_data_chunks - start_chunk;
	
	//Get chunk sizes to sample (one sample per on-screen pixel)
	float	desired_chunk_size = v_pps*(float)AUDIO_CHUNK_SIZE;
	s_time	actual_chunk_size = (s_time)ceil(desired_chunk_size);
	float	actual_to_desired = desired_chunk_size/(float)actual_chunk_size;		//Converts sampled chunk to desired chunk
	

	const AudioVelChunk** pDataVel = (const AudioVelChunk**)((*audioData).data() + start_chunk);
	
	//Number of pixels up until start_chunk
	float start_px = start_chunk*a_ppc;

	//Allocate space for sampled chunks
	AudioChunk	**s_data = new AudioChunk*[num_chunks];
	for(c_time i = 0; i < num_chunks; i++)
		s_data[i] = new AudioChunk(actual_chunk_size);

	//Sample data
	AStatus status = sampleChunks(pDataVel, s_data, num_chunks, SampleMethod::AVERAGE, false);

	if(!statusGood(status))
	{
		std::cout << "AUDIO DISPLAY -->\n" << status << "\n";
		return;
	}

	////DRAW////
	std::vector<TVertex> points;
	Color col;

	//Draw status highlights
	if(drawStatuses)
	{
		for(c_time c = 0; c < num_chunks; c++)
		{
			//Set status color
			switch(pDataVel[c]->getStatus())
			{
			case DataStatus::CLEAN:
				gl.setColor(0.5f, 0.65f, 0.55f);
				break;
			case DataStatus::DIRTY:
				gl.setColor(0.65f, 0.5f, 0.5f);
				break;
			default:
				//Unknown status
				gl.setColor(1.0f, 0.0f, 1.0f);
			}

			//Draw chunk status
			gl.drawRect(APoint((start_chunk + c)*a_ppc, 0.0f), AVec(a_ppc, size.y));
		}

		/*
		DataStatus curr_status = pDataVel[0]->getStatus();
		ChunkRange c_r(-1, -1);

		for(c_time c = 0; c < num_chunks; c++)
		{
			c_time d_c = start_chunk + c;
			DataStatus d_status = pDataVel[c]->getStatus();
			bool continue_chain = (d_status == curr_status);

			if(continue_chain || c == num_chunks - 1)
			{
				//Continue chain
				c_r.start = (c_r.start < 0 ? d_c : c_r.start);
				c_r.end = d_c + 1;
			}

			if(!continue_chain || c == num_chunks - 1)
			{
				//Set status color
				switch(curr_status)
				{
				case DataStatus::CLEAN:
					gl.setColor(0.5f, 0.65f, 0.55f);
					break;
				case DataStatus::DIRTY:
					gl.setColor(0.65f, 0.5f, 0.5f);
					break;
				default:
					//Unknown status
					gl.setColor(1.0f, 0.0f, 1.0f);
				}

				//Draw current chain
				gl.drawRect(APoint(c_r.start*a_ppc, 0.0f), AVec(c_r.length()*a_ppc, size.y));

				//Start new chain
				c_r.start = d_c;
				c_r.end = d_c + 1;
				curr_status = d_status;
			}
		}
		*/
	}

	//Draw center line
	gl.setColor(0.2f, 0.2f, 0.2f, 1.0f);
	gl.drawLine(APoint(start_px, origin.y), APoint(start_px + total_samples*a_pps, origin.y));
	
	//TEST: Draw every data point (to compare approximation with)
	if(Keyboard::keyDown(Keys::K_Q))
	{
		points.clear();
		points.reserve(num_samples);
		col = Color(1.0f, 0.0f, 0.0f, 1.0f);

		const c_time	c_start = (flipChunks ? (num_chunks - 1) : 0),
						c_step = (flipChunks ? -1 : 1);
		const s_time	s_start = (flipChunkSamples ? (AUDIO_CHUNK_SIZE - 1) : 0),
						s_step = (flipChunkSamples ? -1 : 1);

		for(c_time c = c_start; (!flipChunks && c < num_chunks) || (flipChunks && c >= 0); c += c_step)
		{
			const AudioVelSample *c_data = pDataVel[c]->getData();
			AudioVelSample	amplitude = (flipChunkSamples ? pDataVel[c]->getNextSample() : pDataVel[c]->getPreviousSample());

			for(s_time s = s_start; (!flipChunkSamples && s < AUDIO_CHUNK_SIZE) || (flipChunkSamples && s >= 0); s += s_step)
			{
				amplitude += c_data[s]*s_step;

				//Calculate number of samples from the start (adjusting for flipped data or chunks)
				const s_time s_x = (flipChunks ? (num_chunks - 1 - c) : c)*AUDIO_CHUNK_SIZE
								+ (flipChunkSamples ? (AUDIO_CHUNK_SIZE - 1 - s) : s);

				APoint p(start_px + s_x*a_pps, origin.y - origin.y*(float)amplitude*sample_mult);
				points.push_back(TVertex(p, col));
			}
		}
		gl.drawShape(GL_LINE_STRIP, points);
	}

	//Draw lines connecting samples (with approximate, sampled data)
	points.clear();
	points.reserve(num_chunks*desired_chunk_size);
	col = Color(0.0f, 0.0f, 0.0f, 1.0f);

	const c_time	c_start = (flipChunks ? (num_chunks - 1) : 0),
					c_step = (flipChunks ? -1 : 1);
	const s_time	s_start = (flipChunkSamples ? (actual_chunk_size - 1) : 0),
					s_step = (flipChunkSamples ? -1 : 1);
	
	for(c_time c = c_start; (!flipChunks && c < num_chunks) || (flipChunks && c >= 0); c += c_step)
	{
		const AudioSample *c_data = s_data[c]->getData();

		for(s_time s = s_start; (!flipChunkSamples && s < actual_chunk_size) || (flipChunkSamples && s >= 0); s += s_step)
		{
			const s_time s_x = (flipChunks ? (num_chunks - 1 - c) : c)*actual_chunk_size
								+ (flipChunkSamples ? (actual_chunk_size - 1 - s) : s);

			APoint p(start_px + s_x*actual_to_desired*v_to_a, origin.y - origin.y*(float)c_data[s]*sample_mult);
			points.push_back(TVertex(p, col));
		}
	}

	gl.drawShape(GL_LINE_STRIP, points);

	if(DRAW_CHUNK_LINES)
	{
		points.clear();
		points.reserve(2*num_chunks);
		col = Color(0.4f, 0.4f, 0.9f, 0.3f);

		float x = av_pos.x + (AUDIO_CHUNK_SIZE - first_chunk_so)*a_pps;
		
		for(c_time c = 0; c < num_chunks; c++, x += a_ppc)
		{
			if(pDataVel[c]->NOTE_ON)
			{
				//col = Color(1.0f, 0.0f, 0.0f, 0.5f);
				gl.setColor(Color(1.0f, 0.0f, 0.0f, 0.5f));
				gl.drawRect(APoint(x - AUDIO_CHUNK_SIZE*a_pps, 0.0f), AVec(AUDIO_CHUNK_SIZE*a_pps, size.y));
			}
			
			col = Color(0.4f, 0.4f, 0.9f, 0.3f);

			points.push_back(TVertex(APoint(x, 0.0f), col));
			points.push_back(TVertex(APoint(x, size.y), col));
		}

		gl.drawShape(GL_LINES, points);
		points.clear();
	}

	//Draw cursor
	if(cursor)
	{
		float x = cursor_time*a_pps;
		
		gl.setColor(0.8f, 0.8f, 0.1f);
		gl.drawLine(APoint(x, 0.0f), APoint(x, size.y));
	}
	
	//Release sampled data
	if(s_data)
	{
		for(int i = 0; i < num_chunks; i++)
			delete s_data[i];
		delete[] s_data;
	}

}

void AudioDisplay::draw(GlInterface &gl)
{
	if(visible && isolateViewport(gl))
	{
		drawBackground(gl);
		
		if(audioData && audioData->size() > 0)
			((*audioData)[0]->getType() == ChunkType::AUDIO ? drawData(gl) : drawDataVel(gl));

		//(No children)

		restoreViewport(gl);
	}
}





/////MIDI DISPLAY/////

MidiDisplay::MidiDisplay(ParentElement *parent_, APoint a_pos, AVec a_size)
	: CompoundControl(parent_, a_pos, a_size, true, false)//,
		//midiData(midi_data)
{
	setAllBackgroundColors(Color(0.5f, 0.5f, 0.6f, 1.0f));
	
	onSizeChanged(AVec());
}

void MidiDisplay::onSizeChanged(AVec d_size)
{
	if(midiData)
	{
		//Make sure to round up to the nearest chunk
		c_time chunk_length = (c_time)ceil(midiData->getLength()*(double)TEMP_SAMPLERATE/(double)AUDIO_CHUNK_SIZE);
		//std::cout << "MIDI NUM CHUNKS:   " << chunk_length << "\n";

		samplesPerPixel = (float)chunk_length*(float)AUDIO_CHUNK_SIZE/size.x;
		secondsPerPixel = samplesPerPixel/(float)TEMP_SAMPLERATE;
	}
}


void MidiDisplay::setCursor(Cursor *p_cursor)
{
	cursor = p_cursor;
}

void MidiDisplay::setData(const MidiData *midi_data)
{
	midiData = midi_data;
}

const MidiData* MidiDisplay::getData() const
{
	return midiData;
}

void MidiDisplay::drawData(GlInterface &gl)
{
	//(Up here for now to minimize flickering)
	s_time cursor_time = (cursor ? cursor->getSampleRange().start : 0);

	//Get view origin/size in absolute space
	APoint	av_pos = gl.viewToAbsolutePoint(APoint(0, 0));
	AVec	av_size = gl.viewToAbsoluteVec(gl.getCurrView().size);

				//Samples per pixel
	const float a_spp = samplesPerPixel,
				a_pps = 1.0f/a_spp,
				v_spp = gl.absoluteToViewVec(AVec(samplesPerPixel, 0)).x,
				v_pps = 1.0f/v_spp,

				//Chunks per pixel
				a_cpp = a_spp*AUDIO_CHUNK_SIZE,
				a_ppc = a_pps*AUDIO_CHUNK_SIZE,
				v_cpp = v_spp*AUDIO_CHUNK_SIZE,
				v_ppc = v_pps*AUDIO_CHUNK_SIZE,

				//Seconds per pixel
				a_secpp = secondsPerPixel,
				a_ppsec = 1.0f/a_secpp,
				v_secpp = gl.absoluteToViewVec(AVec(secondsPerPixel, 0)).x,
				v_ppsec = 1.0f/v_secpp,

				note_mult = 1.0f/NUM_MIDI_NOTES;	//Normalizes MidiIndex between 0 and 1

	//Left-most point, starting from bottom
	APoint origin(0, size.y);

	//Position adjustment values
	double	t_offset = (double)(av_pos.x*a_secpp),
			t_length = (double)(av_size.x*a_secpp),
			end_t = t_offset + t_length;

	//std::cout << t_offset << "\n";

	TimeRange r(t_offset, end_t);

	//Get intersecting notes
	ConstMidiData midi = midiData->getConstNotes(r);
	

	s_time	sample_offset = (s_time)(av_pos.x*a_spp), 
			num_samples = (s_time)ceil(av_size.x*a_spp),
			first_chunk_so = sample_offset % AUDIO_CHUNK_SIZE,
			last_chunk_so = (first_chunk_so + num_samples) % AUDIO_CHUNK_SIZE,
			total_samples = first_chunk_so + num_samples + last_chunk_so;



	////DRAW////

	std::vector<TVertex> points;
	points.reserve(2*midi.size());
	Color col(0.0f, 0.0f, 0.0f, 1.0f);

	for(auto n : midi)
	{
		float	height = origin.y - origin.y*((float)n->index*note_mult),
				x1 = (float)n->range.start*a_ppsec,
				//x2 = (float)(n->isFinished() ? n->end : (float)cursor_time/TEMP_SAMPLERATE)*a_ppsec;
				x2 = (float)n->range.end*a_ppsec;

		points.push_back(TVertex(APoint(x1, height), col));
		points.push_back(TVertex(APoint(x2, height), col));
	}
	gl.drawShape(GL_LINES, points);

	points.clear();
	col = Color(1.0f, 1.0f, 0.0f, 1.0f);

	//Draw stoppers
	/*
	for(auto n : midi)
	{
		float height = origin.y - origin.y*((float)n->index*note_mult);
		for(auto st : n->stoppers)
		{
			float x = (float)(n->range.start + st.offset)*a_ppsec;
			points.push_back(TVertex(APoint(x, height), col));
		}
	}
	gl.drawShape(GL_POINTS, points);
	*/

	//Draw chunk lines
	if(DRAW_CHUNK_LINES)
	{
		//Convert seconds to chunks
		c_time num_chunks = (c_time)ceil((float)total_samples/(float)AUDIO_CHUNK_SIZE);

		points.clear();
		points.reserve(2*num_chunks);
		col = Color(0.4f, 0.4f, 0.9f, 0.3f);

		float x = av_pos.x + (AUDIO_CHUNK_SIZE - first_chunk_so)*a_pps;

		for(c_time c = 0; c < num_chunks; c++, x += a_ppc)
		{
			points.push_back(TVertex(APoint(x, 0.0f), col));
			points.push_back(TVertex(APoint(x, size.y), col));
		}

		gl.drawShape(GL_LINES, points);
		points.clear();
	}
	
	//Draw cursor
	if(cursor)
	{
		float x = (float)cursor_time/TEMP_SAMPLERATE*a_ppsec;
		
		gl.setColor(0.8f, 0.8f, 0.1f);
		gl.drawLine(APoint(x, 0.0f), APoint(x, size.y));
	}

	//TEST
	//gl.setColor(1.0f, 0.0f, 0.0f);
	//gl.drawLine(APoint(end_t*v_ppsec, 0), APoint(end_t*v_ppsec, size.y));
}

void MidiDisplay::draw(GlInterface &gl)
{
	if(visible && isolateViewport(gl))
	{
		drawBackground(gl);
		if(midiData)
			drawData(gl);

		//(No children)

		restoreViewport(gl);
	}
}