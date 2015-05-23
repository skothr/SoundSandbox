#ifndef APOLLO_OBJ_DESC_H
#define APOLLO_OBJ_DESC_H

#include <fstream>
#include <vector>
#include <memory>

#include "MIDI.h"
#include "WavePoint.h"
#include "FileSystem.h"

#include "AudioData.h"

typedef int NCID;
typedef int NID;

enum class NodeDataType;
enum class NodeConnectionType;
enum class NodeType;

enum class InstrumentType;
enum class ADSRInterpType;

class TrackPoint;
class NodeConnector;
class NodeConnection;
//typedef std::weak_ptr<NodeConnection> NCAccessPtr;
//typedef std::shared_ptr<NodeConnection> NCOwnedPtr;

typedef NodeConnector* NCOwnedPtr;
typedef NodeConnection* COwnedPtr;

//Contains structs used for describing all required information necessary for each type of object
//	Except for vectors, cannot rely on pointers!! (Used for saving data across sessions)

//Base class for object descriptors
struct ObjDesc
{
	//Returns number of bytes read/written
	virtual int writeToFile(std::ofstream &out) = 0;
	virtual int readFromFile(std::ifstream &in) = 0;
};


//Represents an object that can be saved to a file
class Saveable
{
protected:
	ObjDesc* objDesc = nullptr;
	
	virtual void updateDesc() = 0;

public:
	Saveable() {};
	//Saveable(const ObjDesc &obj_desc);
	virtual ~Saveable();

	ObjDesc* getDesc();
	//virtual void loadFromDesc(const ObjDesc &obj_desc) = 0;
};


/////DEVICES/////

struct DevDesc : public ObjDesc
{
	std::string name;

	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct MidiDevDesc : public DevDesc
{
	bool	vDev;
	int		octaveOffset;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct AudioDevDesc : public DevDesc
{
	unsigned int	sampleRate,
					chunkSize,
					numChannels;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


/////WAVEFORM/////

struct WaveformDesc : public ObjDesc
{
	std::vector<WavePoint> points;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


/////INSTRUMENTS/////

struct InstrumentDesc : public ObjDesc
{
	InstrumentType	type;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};

struct WaveSynthDesc : public InstrumentDesc
{
	ADSRInterpType		interp;
	
	double				attackTime,
						decayTime,
						sustainLevel,
						releaseTime;

						//attackK, attackB, attackM,
						//decayK, decayB, decayM,
						//releaseK, releaseB, releaseM;

	WaveformDesc		waveform;

	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


/////CURSOR/////

struct CursorDesc : public ObjDesc
{
	int sampleRate;
	SampleRange s_range;

	s_time chunkSize;
	c_time chunkStep;

	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


/////NODES/////

struct NCDesc : public ObjDesc
{
	NCID				id;
	NID				nodeId;

	NodeDataType		dataType;
	NodeConnectionType	ioType;
	
	int					maxConnections;

	std::string			name,
						desc;

	std::vector<NCOwnedPtr>	connections;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};

struct NDesc : public ObjDesc
{
	NID				id;
	NodeType			type;

	std::string			nameStr,
						typeStr,
						descStr;
	
	bool				dirty;

	std::vector<NCID>	connectors;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct RenderNDesc : public NDesc
{
	int sampleRate;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct TimeMapNDesc : public NDesc
{
	int			sampleRate;
	c_time		maxBufferLength;

	CursorDesc	cursor;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


template<typename T>
struct TrackNDesc : public NDesc
{
	T	length,
		maxLength;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};

template<typename T>
int TrackNDesc<T>::writeToFile(std::ofstream &out)
{
	int n_bytes = 0;

	n_bytes += FileSystem::writeFile(out, length);
	n_bytes += FileSystem::writeFile(out, maxLength);

	return n_bytes;
}
template<typename T>
int TrackNDesc<T>::readFromFile(std::ifstream &in)
{
	int n_bytes = 0;

	n_bytes += FileSystem::readFile(in, length);
	n_bytes += FileSystem::readFile(in, maxLength);

	return n_bytes;
}


struct AudioTrackNDesc : public TrackNDesc<c_time>
{
	int				sampleRate;
	AudioVelData	data;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct MidiTrackNDesc : public TrackNDesc<Time>
{
	int						sampleRate;
	std::vector<MidiNote>	data;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct ModTrackNDesc : TrackNDesc<c_time>
{
	std::vector<TrackPoint> points;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct AudioModTrackNDesc : public ModTrackNDesc
{
	AudioModFunction		modifyFunc;

	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct MidiModTrackNDesc : public ModTrackNDesc
{
	SampleModFunction		modifyFunc;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct InstrumentNDesc : public NDesc
{
	InstrumentDesc		instrument;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct MidiDeviceNDesc : public NDesc
{
	MidiDevDesc			device;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


struct SpeakerNDesc : public NDesc
{
	int					bufferSize,
						numChannels,
						sampleRate;

	bool				requestUpdate;
	
	AudioDevDesc		device;
	
	virtual int writeToFile(std::ofstream &out) override;
	virtual int readFromFile(std::ifstream &in) override;
};


#endif	//APOLLO_OBJ_DESC_H