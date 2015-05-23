#ifndef APOLLO_MIDI_DEVICE_H
#define APOLLO_MIDI_DEVICE_H

#include "RtMidi.h"
#include "MIDI.h"
#include "Timing.h"
#include "AStatus.h"

#include "Keyboard.h"
#include "Events.h"

#include "Saveable.h"

#include <vector>
#include <functional>
#include <unordered_map>

typedef std::function<AStatus(MidiEvent)> MidiCallback;

#define NUM_VIRTUAL_NOTES	18
#define VIRTUAL_BASE_INDEX	60

typedef int MidiPort;

class MidiDevice : public Saveable
{
private:
	static std::vector<MidiDevice*>						virtualDevices;
	//A mapping of each keyboard key to a MidiIndex
	static const std::unordered_map<KeyCode, MidiIndex> vKeyIndexMapping;
	static const unsigned char							virtualVelocity;

	static RtMidiIn *g_midiIn;

	RtMidiIn		*midiIn = nullptr;
	//TODO: Make a class for midi output too (?)

	//Callback 
	MidiCallback	callback = nullptr;

	MidiPort		port = -1;
	std::string		name = "";

	//Whether this device is virtual (computer keyboard)
	bool			vDev = false;
	//Number of octaves offset from center
	int				octaveOffset = 0;

	//The current time of any events
	Time			eventTime = -1.0;
	
	MidiDeviceState	state;

	void updateStates(int sample_offset);

public:

	MidiDevice();
	MidiDevice(const MidiDevDesc &md_desc);
	virtual ~MidiDevice();
	
	static AStatus initDevices();
	static unsigned int numPorts();
	static AStatus printDevices();

	//Returns current port of the device with the specified name (or -1 if it isn't currently connected)
	static int getPort(std::string dev_name);

	static AStatus keyEvent(KeyCode key, EventType key_event);
	
	AStatus init(MidiPort p, MidiCallback callback_func);

	void raiseOctave();
	void lowerOctave();

	bool isConnected();
	
	MidiPort getPort();
	std::string getName();
	
	static void midiCallback(double time_stamp, std::vector<unsigned char> *message, void *p_device);

	void setCallback(MidiCallback cb_func);

	const MidiDeviceState* getState() const;
	
protected:
	virtual void updateDesc() override;
};

#endif	//APOLLO_MIDI_DEVICE_H