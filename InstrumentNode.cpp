#include "InstrumentNode.h"

#include "NodePackets.h"
#include "AUtility.h"

/////INSTRUMENT NODE/////
const std::vector<NodeConnectorDesc>	InstrumentNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::INFO, IOType::INFO_OUTPUT, "Instrument Output", "Instrument sampling data.", -1) };

WaveSynth InstrumentNode::defaultInstrument;

InstrumentNode::InstrumentNode(NodeGraph *parent_graph)
	: InstrumentNode(parent_graph, &defaultInstrument, true)
{ }

/*
InstrumentNode::InstrumentNode(const InstrumentNDesc &in_desc)
	: Node(*dynamic_cast<const NDesc*>(&in_desc)),
		instrument(nullptr), needDelete(true)
{
	Node::init(in_desc.connectors);
	OUTPUTS.INSTRUMENT_ID = in_desc.connectors[0];

	switch(in_desc.instrument.type)
	{
	case InstrumentType::WAVESYNTH:
		instrument = new WaveSynth(*(WaveSynthDesc*)&in_desc.instrument);
		break;
	}
}
*/

InstrumentNode::InstrumentNode(NodeGraph *parent_graph, Instrument *instrument_, bool cpy_instrument)
	: Node(parent_graph, NType::INSTRUMENT, "Instrument Node", "Holds a description of an instrument for use with MIDI data."),
		instrument(instrument_), needDelete(cpy_instrument && instrument_)
{
	initNode();

	if(cpy_instrument && instrument_)
	{
		switch(instrument_->getType())
		{
		case InstrumentType::WAVESYNTH:
			instrument = new WaveSynth();
			break;
		default:
			break;
		}
	}
	//else instrument is null --> keep null
}

InstrumentNode::~InstrumentNode()
{
	if(needDelete)
		AU::safeDelete(instrument);
}


void InstrumentNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);

	OUTPUTS.INSTRUMENT_ID = ids[0];
}

Instrument* InstrumentNode::getInstrument()
{
	return instrument;
}

//bool InstrumentNode::canPull()
//{
//	return static_cast<bool>(instrument);
//}
//
//bool InstrumentNode::canFlush()
//{
//	return false;
//}

bool InstrumentNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	//Check output
	if(this_id == OUTPUTS.INSTRUMENT_ID)	
	{
		InstrumentPacket *inst_output = dynamic_cast<InstrumentPacket*>(&output);

		if(inst_output && instrument)
		{
			inst_output->sample = instrument->getSampleFunction();
			inst_output->sampleVel = instrument->getSampleVelFunction();
			return true;
		}
	}

	return false;
}

/*
void InstrumentNode::updateDesc()
{
	Node::updateDesc();
	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new InstrumentNDesc()));
	InstrumentNDesc *desc = dynamic_cast<InstrumentNDesc*>(objDesc);
	
	desc->instrument = *(InstrumentDesc*)instrument->getDesc();
}
*/