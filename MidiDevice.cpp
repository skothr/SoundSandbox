#include "MidiDevice.h"

#include "AUtility.h"
#include <iomanip>

/////MIDI DEVICE/////
RtMidiIn *MidiDevice::g_midiIn = nullptr;

std::vector<MidiDevice*>						MidiDevice::virtualDevices;
const unsigned char								MidiDevice::virtualVelocity = 30;
const std::unordered_map<KeyCode, MidiIndex>	MidiDevice::vKeyIndexMapping {	{Keys::K_A, VIRTUAL_BASE_INDEX + 0},
																				{Keys::K_W, VIRTUAL_BASE_INDEX + 1},
																				{Keys::K_S, VIRTUAL_BASE_INDEX + 2},
																				{Keys::K_E, VIRTUAL_BASE_INDEX + 3},
																				{Keys::K_D, VIRTUAL_BASE_INDEX + 4},
																				{Keys::K_F, VIRTUAL_BASE_INDEX + 5},
																				{Keys::K_T, VIRTUAL_BASE_INDEX + 6},
																				{Keys::K_G, VIRTUAL_BASE_INDEX + 7},
																				{Keys::K_Y, VIRTUAL_BASE_INDEX + 8},
																				{Keys::K_H, VIRTUAL_BASE_INDEX + 9},
																				{Keys::K_U, VIRTUAL_BASE_INDEX + 10},
																				{Keys::K_J, VIRTUAL_BASE_INDEX + 11},
																				{Keys::K_K, VIRTUAL_BASE_INDEX + 12},
																				{Keys::K_O, VIRTUAL_BASE_INDEX + 13},
																				{Keys::K_L, VIRTUAL_BASE_INDEX + 14},
																				{Keys::K_P, VIRTUAL_BASE_INDEX + 15},
																				{Keys::K_SEMICOLON, VIRTUAL_BASE_INDEX + 16},
																				{Keys::K_QUOTE, VIRTUAL_BASE_INDEX + 17}
																			};


MidiDevice::MidiDevice()
{ }

/*
MidiDevice::MidiDevice(const MidiDevDesc &md_desc)
{
	///Init new MidiDevice///
	init((md_desc.vDev ? -1 : getPort(md_desc.name)), nullptr);
}
*/

MidiDevice::~MidiDevice()
{
	if(vDev)
	{
		//Remove from global list
		for(unsigned int i = 0; i < virtualDevices.size(); i++)
		{
			if(virtualDevices[i] == this)
				virtualDevices.erase(virtualDevices.begin() + i);
		}
	}

	if(midiIn)
	{
		//Close port
		try
		{
			midiIn->closePort();
		}
		catch(RtMidiError &error)
		{
			std::cout << "Error closing MIDI device port!\n";
		}

		delete midiIn;
	}
	midiIn = nullptr;
}


bool MidiDevice::initDevices()
{
	bool status = true;

	if(!g_midiIn)
	{
		//Init RtMidiIn for global querying of devices
		try
		{
			g_midiIn = new RtMidiIn();
		}
		catch(RtMidiError &error)
		{
			std::cout << "Error initializing global MIDI device!  -->  " << error.getMessage() << "\n";
			status = false;
		}
	}
	else
	{
		std::cout << "MidiDevice has already been initialized.\n\n";
		status = false;
	}
	
	return status;
}

void MidiDevice::cleanupDevices()
{
	for(auto vd : virtualDevices)
		if(vd)
			delete vd;
	virtualDevices.clear();

	if(g_midiIn)
		delete g_midiIn;
	g_midiIn = nullptr;
}

unsigned int MidiDevice::numPorts()
{
	return (g_midiIn ? g_midiIn->getPortCount() : 0u);
}

bool MidiDevice::printDevices()
{
	bool status = true;

	if(g_midiIn)
	{
		//Check available ports
		unsigned int n_ports = numPorts();

		if(n_ports != 1)
			std::cout << "\nThere are " << n_ports << " MIDI input sources available.\n";
		else
			std::cout << "\nThere is " << n_ports << " MIDI input source available.\n";

		if(n_ports != 0)
			std::cout << "\n------------------------\nMIDI Devices:\n";
	
		std::string port_name;

		for(unsigned int i = 0; i < n_ports; i++)
		{
			try
			{
				port_name = g_midiIn->getPortName(i);
			}
			catch(RtMidiError &error)
			{
				std::cout << "Error getting MIDI device port name!  -->  " << error.getMessage() << "\n";
				status = false;
				break;
			}
			std::cout << "\t" << i << ":\t" << port_name << "\n";
		}
	}
	else
	{
		std::cout << "Error printing devices: MidiDevice has yet to be sucessfully initialized!\n";
	}

	return status;
}

int MidiDevice::getPort(std::string dev_name)
{
	unsigned int n_ports = numPorts();

	for(unsigned int i = 0; i < n_ports; i++)
	{
		try
		{
			if(g_midiIn->getPortName(i) == dev_name)
				return i;
		}
		catch(RtMidiError &error)
		{
			continue;
		}
	}

	return -1;
}

bool MidiDevice::keyEvent(KeyCode key, EventType key_event)
{
	bool status = true;
	
	Time event_time = Clock::getGlobalTime();

	auto found = vKeyIndexMapping.find(key);
	if(found != vKeyIndexMapping.end())
	{
		for(auto vd : virtualDevices)
		{
			MidiIndex m_index = found->second + vd->octaveOffset*OCTAVE_SIZE;

			if(m_index > NUM_MIDI_NOTES - 1)
			{
				std::cout << "NOTE TOO HIGH!\n";
				continue;
			}
			else if(m_index < 0)
			{
				std::cout << "NOTE TOO LOW!\n";
				continue;
			}

			std::vector<unsigned char> message(3);

			switch(key_event)
			{
			case Events::KEY_DOWN:
				message[0] = getByte(MidiEventType::NOTE_ON);
				break;
			case Events::KEY_UP:
				message[0] = getByte(MidiEventType::NOTE_OFF);
				break;
			default:
				continue;
			}

			//TODO: implement channels (?)
			message[0] |= getByte(MidiChannel::CHANNEL1);

			message[1] = (unsigned char)m_index;
			message[2] = virtualVelocity;

			double time_stamp = (vd->eventTime < 0.0 ? 0.0 : (event_time - vd->eventTime));
			vd->midiCallback(time_stamp, &message, vd);
		}
	}
	else if(key == Keys::K_UPARROW)
	{
		for(auto vd : virtualDevices)
			vd->raiseOctave();
	}
	else if(key == Keys::K_DOWNARROW)
	{
		for(auto vd : virtualDevices)
			vd->lowerOctave();
	}

	return status;
}


bool MidiDevice::init(MidiPort p, MidiCallback callback_func)
{
	bool status = true;

	//if(port == p)
	//	status.setWarning(AS::WType::NO_ACTION_TAKEN, "The specified port was already loaded on this MidiDevice.");

	//Unload existing connected midi device//
	if(midiIn)
		delete midiIn;
	midiIn = nullptr;

	//Unload existing virtual device//
	if(vDev)
	{
		vDev = false;
		auto found = std::find(virtualDevices.begin(), virtualDevices.end(), this);
		if(found != virtualDevices.end())
			virtualDevices.erase(found);
	}
	
	//Create new device//

	port = p;
	callback = callback_func;

	if(p < 0)
	{
		//Create virtual device
		vDev = true;
		virtualDevices.push_back(this);
	}
	else
	{
		//Set up rtmidi device
		try
		{
			midiIn = new RtMidiIn();
		}
		catch(RtMidiError &error)
		{
			std::cout << "Error creating MIDI device  -->  " <<  error.getMessage() << "\n";
			return false;
		}

		unsigned int n_ports = numPorts();

		try
		{
			name = midiIn->getPortName(p);
		}
		catch(RtMidiError &error)
		{
			std::cout << "Error getting MIDI device port name!  -->  " << error.getMessage() << "\n";
			return false;
		}

		//Open port
		try
		{
			midiIn->openPort(port);
		}
		catch(RtMidiError &error)
		{
			std::cout <<  "MIDI port failed to open!\n";
			return false;
		}

		//Don't ignore sysex, timing, or active sensing messages
		try
		{
			midiIn->ignoreTypes(false, false, false);
		}
		catch(RtMidiError &error)
		{
			std::cout <<  "Setting MIDI device ignore types failed!\n";
			return false;
		}

		//Set callback function
		try
		{
			midiIn->setCallback(&midiCallback, (void*)this);
		}
		catch(RtMidiError &error)
		{
			std::cout <<  "Setting MIDI callback failed!\n";
			return false;
		}
	}

	return status;
}



void MidiDevice::raiseOctave()
{
	octaveOffset++;
}

void MidiDevice::lowerOctave()
{
	octaveOffset--;
}

bool MidiDevice::isConnected()
{
	return true;
}

MidiPort MidiDevice::getPort()
{
	return port;
}

std::string MidiDevice::getName()
{
	return name;
}


void MidiDevice::midiCallback(double time_stamp, std::vector<unsigned char> *message, void *p_device)
{
	MidiDevice *p_dev = (MidiDevice*)p_device;
	static const Time chunk_time = (Time)AUDIO_CHUNK_SIZE/44100.0;

	if(p_dev)
	{
		//Update time
		Time	g_time = HRes_Clock::getGlobalTime();
		//Add time_stamp (dt) to eventTime, and make sure it isnt ahead of g_time
		//p_dev->eventTime = (p_dev->eventTime < 0.0 ? g_time : min(g_time, p_dev->eventTime + time_stamp));
		p_dev->eventTime = g_time;//(p_dev->eventTime < 0.0 ? g_time : p_dev->eventTime + time_stamp);
		Time	diff = p_dev->eventTime - g_time;
		
		if(abs(diff) > chunk_time)
			std::cout << "OUT OF SYNC!\n";
		
		//Size of message
		unsigned int n_bytes = message->size();

		if(n_bytes > 0)
		{
			MidiEvent e(p_dev->eventTime, *message);
			//Pass event to the appropriate device
			if(p_dev->callback)
			{
				bool status = p_dev->callback(e);
				if(!status)
					std::cout << "MIDI CALLBACK FAILED.\n";
			}

			//Update device states
			p_dev->state.applyEvent(e);
		}
		else
		{
			std::cout << "MESSAGE CONTAINED NO BYTES!\n";
			return;
		}
	}
}

void MidiDevice::setCallback(MidiCallback cb_func)
{
	callback = cb_func;
}


const MidiDeviceState* MidiDevice::getState() const
{
	return &state;
}

/*
void MidiDevice::updateDesc()
{
	objDesc = (objDesc ? objDesc : (ObjDesc*)(new MidiDevDesc()));
	MidiDevDesc *desc = dynamic_cast<MidiDevDesc*>(objDesc);
	
	desc->vDev = vDev;
	desc->name = name;
	desc->octaveOffset = octaveOffset;
}
*/
