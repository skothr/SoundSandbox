#ifndef APOLLO_INSTRUMENT_NODE_H
#define APOLLO_INSTRUMENT_NODE_H

#include <vector>

#include "NodeBaseTypes.h"
#include "Instrument.h"
#include "MidiDevice.h"

class InstrumentNode : public InputNode
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

	static WaveSynth defaultInstrument;

protected:
	Instrument	*instrument;
	//Whether the instrument needs to be deleted 
	bool		needDelete;

public:
	InstrumentNode(NodeGraph *parent_graph);
	//cpy_instrument denotes whether the given instrument should be copied or just used as the given pointer
	InstrumentNode(NodeGraph *parent_graph, Instrument *instrument_, bool cpy_instrument);
	//InstrumentNode(const InstrumentNDesc &in_desc);
	virtual ~InstrumentNode();

	//Connector ids
	struct
	{ } INPUTS;
	struct
	{
		NCID INSTRUMENT_ID = 0;
	} OUTPUTS;

	Instrument* getInstrument();
	
	//virtual bool canPull() override;
	//virtual bool canFlush() override;

	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	
protected:
	//virtual void updateDesc() override;
};


#endif	//APOLLO_INSTRUMENT_NODE_H