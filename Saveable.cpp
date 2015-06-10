#include "Saveable.h"
#include "AUtility.h"

#include "Node.h"
#include "NodeBaseTypes.h"

#include "MidiDevice.h"
#include "AudioDevice.h"
#include "Waveform.h"
#include "Instrument.h"

#include "AudioChunk.h"
/*
/////SAVEABLE/////

//Saveable::Saveable(const ObjDesc &obj_desc)
//{
//	loadFromDesc(obj_desc);
//}

Saveable::~Saveable()
{
	AU::safeDelete(objDesc);
}

ObjDesc* Saveable::getDesc()
{
	updateDesc();
	return objDesc;
}




/////DEVICES/////

int DevDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = FileSystem::writeFileString(out, name);

	return n_bytes;
}
int DevDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = FileSystem::readFileString(in, name);

	return n_bytes;
}

int MidiDevDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = DevDesc::writeToFile(out);

	n_bytes += FileSystem::writeFile(out, vDev);
	n_bytes += FileSystem::writeFile(out, octaveOffset);

	return n_bytes;
}
int MidiDevDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = DevDesc::readFromFile(in);
	
	n_bytes += FileSystem::readFile(in, vDev);
	n_bytes += FileSystem::readFile(in, octaveOffset);

	return n_bytes;
}

int AudioDevDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = DevDesc::writeToFile(out);

	n_bytes += FileSystem::writeFile(out, sampleRate);
	n_bytes += FileSystem::writeFile(out, chunkSize);
	n_bytes += FileSystem::writeFile(out, numChannels);

	return n_bytes;
}
int AudioDevDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = DevDesc::readFromFile(in);

	n_bytes += FileSystem::readFile(in, sampleRate);
	n_bytes += FileSystem::readFile(in, chunkSize);
	n_bytes += FileSystem::readFile(in, numChannels);

	return n_bytes;
}



/////WAVEFORM/////

int WaveformDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = 0;

	//Number of points
	int num_points = (int)points.size();
	n_bytes += FileSystem::writeFile(out, num_points);

	for(int i = 0; i < num_points; i++)
	{
		n_bytes += FileSystem::writeFile(out, points[i].interp_type);
		n_bytes += FileSystem::writeFile(out, points[i].x);
		n_bytes += FileSystem::writeFile(out, points[i].y);
	}

	return n_bytes;
}
int WaveformDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = 0,
		num_points = 0;

	//Number of points
	n_bytes += FileSystem::readFile(in, num_points);
	points.resize(num_points);

	for(int i = 0; i < num_points; i++)
	{
		n_bytes += FileSystem::readFile(in, points[i].interp_type);
		n_bytes += FileSystem::readFile(in, points[i].x);
		n_bytes += FileSystem::readFile(in, points[i].y);
	}

	return n_bytes;
}


/////INSTRUMENTS/////

int InstrumentDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = FileSystem::writeFile(out, type);

	return n_bytes;
}
int InstrumentDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = FileSystem::readFile(in, type);

	return n_bytes;
}

int WaveSynthDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = InstrumentDesc::writeToFile(out);

	n_bytes += FileSystem::writeFile(out, attackTime);
	n_bytes += FileSystem::writeFile(out, decayTime);
	n_bytes += FileSystem::writeFile(out, sustainLevel);
	n_bytes += FileSystem::writeFile(out, releaseTime);

	n_bytes += waveform.writeToFile(out);

	return n_bytes;
}
int WaveSynthDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = InstrumentDesc::readFromFile(in);
	
	n_bytes += FileSystem::readFile(in, attackTime);
	n_bytes += FileSystem::readFile(in, decayTime);
	n_bytes += FileSystem::readFile(in, sustainLevel);
	n_bytes += FileSystem::readFile(in, releaseTime);

	n_bytes += waveform.readFromFile(in);

	return n_bytes;
}



/////CURSOR/////

int CursorDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = 0;

	n_bytes += FileSystem::writeFile(out, sampleRate);

	n_bytes += FileSystem::writeFile(out, s_range.start);
	n_bytes += FileSystem::writeFile(out, s_range.end);

	n_bytes += FileSystem::writeFile(out, chunkSize);
	n_bytes += FileSystem::writeFile(out, chunkStep);

	return n_bytes;
}

int CursorDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = 0;

	n_bytes += FileSystem::readFile(in, sampleRate);

	n_bytes += FileSystem::readFile(in, s_range.start);
	n_bytes += FileSystem::readFile(in, s_range.end);

	n_bytes += FileSystem::readFile(in, chunkSize);
	n_bytes += FileSystem::readFile(in, chunkStep);

	return n_bytes;
}



/////NODES/////

int NCDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = 0;

	n_bytes += FileSystem::writeFile(out, id);
	n_bytes += FileSystem::writeFile(out, nodeId);
	n_bytes += FileSystem::writeFile(out, dataType);
	n_bytes += FileSystem::writeFile(out, ioType);
	n_bytes += FileSystem::writeFile(out, maxConnections);

	n_bytes += FileSystem::writeFileString(out, name);
	n_bytes += FileSystem::writeFileString(out, desc);

	int num_connections = connections.size();
	n_bytes += FileSystem::writeFile(out, num_connections);

	for(int i = 0; i < num_connections; i++)
		n_bytes += FileSystem::writeFile(out, connections[i]);

	return n_bytes;
}
int NCDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = 0,
		num_connections = 0;

	n_bytes += FileSystem::readFile(in, id);
	n_bytes += FileSystem::readFile(in, nodeId);
	n_bytes += FileSystem::readFile(in, dataType);
	n_bytes += FileSystem::readFile(in, ioType);
	n_bytes += FileSystem::readFile(in, maxConnections);

	n_bytes += FileSystem::readFileString(in, name);
	n_bytes += FileSystem::readFileString(in, desc);

	n_bytes += FileSystem::readFile(in, num_connections);
	connections.resize(num_connections);

	for(int i = 0; i < num_connections; i++)
		n_bytes += FileSystem::readFile(in, connections[i]);

	return n_bytes;
}

int NDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = 0;

	n_bytes += FileSystem::writeFile(out, id);
	n_bytes += FileSystem::writeFile(out, type);

	n_bytes += FileSystem::writeFileString(out, nameStr);
	n_bytes += FileSystem::writeFileString(out, typeStr);
	n_bytes += FileSystem::writeFileString(out, descStr);

	n_bytes += FileSystem::writeFile(out, dirty);
	
	int num_connectors = connectors.size();

	for(int i = 0; i < num_connectors; i++)
		n_bytes += FileSystem::writeFile(out, connectors[i]);

	return n_bytes;
}
int NDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = 0,
		num_connectors = 0;

	n_bytes += FileSystem::readFile(in, id);
	n_bytes += FileSystem::readFile(in, type);

	n_bytes += FileSystem::readFileString(in, nameStr);
	n_bytes += FileSystem::readFileString(in, typeStr);
	n_bytes += FileSystem::readFileString(in, descStr);

	n_bytes += FileSystem::readFile(in, dirty);

	n_bytes += FileSystem::readFile(in, num_connectors);
	connectors.resize(num_connectors);

	for(int i = 0; i < num_connectors; i++)
		n_bytes += FileSystem::readFile(in, connectors[i]);

	return n_bytes;
}

int RenderNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = NDesc::writeToFile(out);

	n_bytes += FileSystem::writeFile(out, sampleRate);

	return n_bytes;
}
int RenderNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = NDesc::readFromFile(in);

	n_bytes += FileSystem::readFile(in, sampleRate);

	return n_bytes;
}

int TimeMapNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = NDesc::writeToFile(out);

	n_bytes += FileSystem::writeFile(out, sampleRate);
	n_bytes += FileSystem::writeFile(out, maxBufferLength);

	n_bytes += cursor.writeToFile(out);

	return n_bytes;
}
int TimeMapNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = NDesc::readFromFile(in);

	n_bytes += FileSystem::readFile(in, sampleRate);
	n_bytes += FileSystem::readFile(in, maxBufferLength);

	n_bytes += cursor.readFromFile(in);

	return n_bytes;
}

int AudioTrackNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = TrackNDesc<c_time>::writeToFile(out);

	n_bytes += FileSystem::writeFile(out, sampleRate);
	
	int num_chunks = data.getNumChunks();
	n_bytes += FileSystem::writeFile(out, num_chunks);

	for(int c = 0; c < num_chunks; c++)
	{
		int num_samples = data.getChunk(c)->getSize();
		n_bytes += FileSystem::writeFile(out, num_samples);

		const AudioVelSample *s_data = data.getChunk(c)->getData()->data();

		for(int s = 0; s < num_samples; s++)
			n_bytes += FileSystem::writeFile(out, s_data[s]);
	}

	return n_bytes;
}
int AudioTrackNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = TrackNDesc<c_time>::readFromFile(in),
		num_chunks = 0;

	n_bytes += FileSystem::readFile(in, sampleRate);
	n_bytes += FileSystem::readFile(in, num_chunks);

	data.resize(num_chunks);

	std::vector<AudioVelChunk*> *c_data = data.getData();

	for(int c = 0; c < num_chunks; c++)
	{
		int num_samples = 0;
		n_bytes += FileSystem::readFile(in, num_samples);

		AudioVelChunk *chunk = data.getChunk(c);

		for(int s = 0; s < num_samples; s++)
			n_bytes += FileSystem::readFile(in, (*chunk->getDataRef())[s]);
	}

	return n_bytes;
}

int MidiTrackNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = NDesc::writeToFile(out);

	n_bytes += FileSystem::writeFile(out, sampleRate);

	int num_notes = data.size();
	n_bytes += FileSystem::writeFile(out, num_notes);

	for(int i = 0; i < num_notes; i++)
	{
		n_bytes += FileSystem::writeFile(out, data[i].index);
		n_bytes += FileSystem::writeFile(out, data[i].velocity);
		n_bytes += FileSystem::writeFile(out, data[i].range.start);
		n_bytes += FileSystem::writeFile(out, data[i].range.end);
	}

	return n_bytes;
}
int MidiTrackNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = NDesc::readFromFile(in),
		num_notes = 0;

	n_bytes += FileSystem::readFile(in, sampleRate);
	
	n_bytes += FileSystem::readFile(in, num_notes);
	data.resize(num_notes);

	for(int i = 0; i < num_notes; i++)
	{
		n_bytes += FileSystem::readFile(in, data[i].index);
		n_bytes += FileSystem::readFile(in, data[i].velocity);
		n_bytes += FileSystem::readFile(in, data[i].range.start);
		n_bytes += FileSystem::readFile(in, data[i].range.end);

		data[i].finished = true;
	}

	return n_bytes;
}

int ModTrackNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = NDesc::writeToFile(out),
		num_points = points.size();

	n_bytes += FileSystem::writeFile(out, num_points);

	for(int i = 0; i < num_points; i++)
	{
		n_bytes += FileSystem::writeFile(out, points[i].time);
		n_bytes += FileSystem::writeFile(out, points[i].value);
		n_bytes += FileSystem::writeFile(out, points[i].interp);

		//Write point data if it has data instead of interpolation
		if(points[i].interp == InterpolationType::DATA)
		{
			int num_data = points[i].data.size();
			n_bytes += FileSystem::writeFile(out, num_data);

			for(int d = 0; d < num_data; d++)
				n_bytes += FileSystem::writeFile(out, points[i].data[d]);
		}
	}

	return n_bytes;
}
int ModTrackNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = NDesc::readFromFile(in),
		num_points = 0;

	n_bytes += FileSystem::readFile(in, num_points);
	
	for(int i = 0; i < num_points; i++)
	{
		n_bytes += FileSystem::readFile(in, points[i].time);
		n_bytes += FileSystem::readFile(in, points[i].value);
		n_bytes += FileSystem::readFile(in, points[i].interp);
		
		//Read point data if it has data instead of interpolation
		if(points[i].interp == InterpolationType::DATA)
		{
			int num_data = 0;
			n_bytes += FileSystem::readFile(in, num_data);

			points[i].data.resize(num_data);

			for(int d = 0; d < num_data; d++)
				n_bytes += FileSystem::readFile(in, points[i].data[d]);
		}
	}

	return n_bytes;
}

int AudioModTrackNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = ModTrackNDesc::writeToFile(out);

	//TODO: How to save modify function??

	return n_bytes;
}
int AudioModTrackNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = ModTrackNDesc::readFromFile(in);
	
	//TODO: How to save modify function??

	return n_bytes;
}

int MidiModTrackNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = ModTrackNDesc::writeToFile(out);
	
	//TODO: How to save modify function??

	return n_bytes;
}
int MidiModTrackNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = ModTrackNDesc::readFromFile(in);
	
	//TODO: How to save modify function??

	return n_bytes;
}

int InstrumentNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = NDesc::writeToFile(out);

	n_bytes += instrument.writeToFile(out);

	return n_bytes;
}
int InstrumentNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = NDesc::readFromFile(in);

	n_bytes += instrument.readFromFile(in);

	return n_bytes;
}

int MidiDeviceNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = NDesc::writeToFile(out);

	n_bytes += device.writeToFile(out);

	return n_bytes;
}
int MidiDeviceNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = NDesc::readFromFile(in);
	
	n_bytes += device.readFromFile(in);

	return n_bytes;
}

int SpeakerNDesc::writeToFile(std::ofstream &out)
{
	int n_bytes = NDesc::writeToFile(out);
	
	n_bytes += FileSystem::writeFile(out, bufferSize);
	n_bytes += FileSystem::writeFile(out, numChannels);
	n_bytes += FileSystem::writeFile(out, sampleRate);
	n_bytes += FileSystem::writeFile(out, requestUpdate);

	n_bytes += device.writeToFile(out);

	return n_bytes;
}
int SpeakerNDesc::readFromFile(std::ifstream &in)
{
	int n_bytes = NDesc::readFromFile(in);
	
	n_bytes += FileSystem::readFile(in, bufferSize);
	n_bytes += FileSystem::readFile(in, numChannels);
	n_bytes += FileSystem::readFile(in, sampleRate);
	n_bytes += FileSystem::readFile(in, requestUpdate);

	n_bytes += device.readFromFile(in);

	return n_bytes;
}
*/