#include "MidiData.h"


/////MIDI DATA/////

MidiData::MidiData()
	: numNotes(0), span(0.0, 0.0), midiLock("Midi Data")
{ }

MidiData::MidiData(const std::vector<MidiNote> &initial_data)
	: numNotes(0), span(0.0, 0.0), midiLock("Midi Data")
{
	midiLock.lockWait();
	addNotes(initial_data);
	midiLock.unlock();
}

MidiData::MidiData(const std::vector<MidiNote*> &initial_data)
	: numNotes(0), span(0.0, 0.0), midiLock("Midi Data")
{
	midiLock.lockWait();
	addNotes(initial_data);
	midiLock.unlock();
}

MidiData::MidiData(const MidiData &other)
	: numNotes(other.numNotes), span(other.span), midiLock("Midi Data")
{
	midiLock.lockWait();

	for(MidiIndex mi = 0; mi < NUM_MIDI_NOTES; mi++)
	{
		auto iter = other.data.find(mi);

		if(iter != other.data.end())
		{
			data[mi] = std::vector<MidiNote*>();
			data[mi].reserve(iter->second.size());

			for(auto n : data[mi])
				data[mi].push_back(new MidiNote(*n));
		}
	}

	span = other.span;

	midiLock.unlock();
}

MidiData::~MidiData()
{
	midiLock.lockWait();

	for(auto d : data)
	{
		for(auto n : d.second)
			if(n) delete n;
		
		d.second.clear();
	}
	data.clear();

	midiLock.unlock();
}

MidiData& MidiData::operator=(const MidiData &other)
{
	midiLock.lockWait();
	clearNotes();

	for(int mi = -1; mi < NUM_MIDI_NOTES; mi++)
	{
		auto iter = other.data.find(mi);

		if(iter != other.data.end())
		{
			data.emplace(mi, std::vector<MidiNote*>());

			data[mi].reserve(iter->second.size());

			for(auto n : iter->second)
				data[mi].push_back(new MidiNote(*n));
		}
	}

	numNotes = other.numNotes;

	span = other.span;

	midiLock.unlock();

	return *this;
}

int MidiData::getNumNotes() const
{
	return numNotes;
}
TimeRange MidiData::getSpan() const
{
	return span;
}
Time MidiData::getLength() const
{
	return (lengthOverride < 0.0 ? span.length() : lengthOverride);
}

MidiNote* MidiData::addNote(const MidiNote &note)
{
	MidiNote *new_note = new MidiNote(note);

	midiLock.lockWait();

	auto iter = data.find(new_note->index);
	if(iter == data.end())
		data.emplace(new_note->index, std::vector<MidiNote*>());

	
	std::vector<MidiNote*> &mi_notes = data[new_note->index];

	if(mi_notes.size() < 1)
	{
		mi_notes.push_back(new_note);
		numNotes++;
	}
	else
	{

		//Put note in the proper place -- iterate backwards since its more likely to be near the end
		bool inserted = false;
		for(int i = mi_notes.size() - 1; i >= 0; i--)
		{
			if(new_note->range.end >= mi_notes[i]->range.end)
			{
				if(new_note->range.start >= mi_notes[i]->range.end)
				{
					//Insert the note here in the list
					//mi_notes.insert(mi_notes.begin() + i, new_note);
					//numNotes++;
					//break;
					continue;
				}
				else
				{
					//This note is overlapping the existing note --> clamp existing note
					if(new_note->range.start <= mi_notes[i]->range.start)
					{
						//Completely covers the existing note, so get rid of it and loop.
						if(mi_notes[i])
							delete mi_notes[i];
						//Erase previous note
						mi_notes.erase(mi_notes.begin() + i);
						//Add new note
						//mi_notes.insert(mi_notes.begin() + i, new_note);
						//inserted = true;
						//break;
						numNotes--;
						continue;
					}
					else
					{
						//Cuts the existing note --> new note takes precedence
						mi_notes[i]->range.end = new_note->range.start;
						mi_notes.insert(mi_notes.begin() + i + 1, new_note);
						numNotes++;
						inserted = true;
						break;
					}
				}
			}
			else
			{
				if(new_note->range.end > mi_notes[i]->range.start)
				{
					//Beginning of the existing note is cut off
					if(new_note->range.start <= mi_notes[i]->range.start)
					{
						//Whole beginning is cut off
						mi_notes[i]->range.start = new_note->range.end;
						//mi_notes.insert(mi_notes.begin() + i - 1, new_note);	//Insert before existing note
						//numNotes++;
						continue;
					}
					else
					{
						//Existing note is split in two -- need to create another new note
						MidiNote *split_note = new MidiNote(mi_notes[i]->index, mi_notes[i]->velocity, mi_notes[i]->range.start, new_note->range.start);
						mi_notes[i]->range.start = new_note->range.end;
						mi_notes.insert(mi_notes.begin() + i, split_note);	//Insert new split note before current note
						mi_notes.insert(mi_notes.begin() + i + 1, new_note);		//Insert new note after split note
						numNotes += 2;
						inserted = true;
						break;
					}
				}
				else
				{
					//New note comes completely before this note
					continue;
				}
			}
		}

		if(!inserted)
			mi_notes.push_back(new_note);
	}

	span.start = std::min(span.start, mi_notes[0]->range.start);
	span.end = std::max(span.end, mi_notes[mi_notes.size() - 1]->range.end);

	midiLock.unlock();

	return new_note;
}

std::vector<MidiNote*> MidiData::addNotes(const std::vector<MidiNote> &new_notes)
{
	std::vector<MidiNote*> p_new_notes;
	p_new_notes.reserve(new_notes.size());

	for(auto n : new_notes)
		p_new_notes.push_back(addNote(n));

	return p_new_notes;
}

std::vector<MidiNote*> MidiData::addNotes(const std::vector<MidiNote*> &new_notes)
{
	std::vector<MidiNote*> p_new_notes;
	p_new_notes.reserve(new_notes.size());

	for(auto n : new_notes)
		p_new_notes.push_back(addNote(*n));

	return p_new_notes;
}

void MidiData::setLength(Time new_length)
{
	lengthOverride = new_length;
}

//Returns true if successful.
bool MidiData::removeNote(MidiNote *note)
{
	std::vector<MidiNote*> &mi_notes = data[note->index];

	auto iter = std::find(mi_notes.begin(), mi_notes.end(), note);

	if(iter != mi_notes.end())
	{
		TimeRange removed_range = (*iter)->range;
		span.start = (iter == mi_notes.begin()) ? (*(iter + 1))->range.start : span.start;
		span.end = (iter == mi_notes.end() - 1) ? (*(iter - 1))->range.end : span.end;

		mi_notes.erase(iter);
	}
	else
		return false;
}

void MidiData::clearNotes()
{
	for(auto d : data)
	{
		for(auto n : d.second)
			if(n) delete n;

		d.second.clear();
	}
	data.clear();

	span.start = 0.0;
	span.end = 0.0;
	numNotes = 0;
}

MidiMap* MidiData::getNotes()
{
	return &data;
}
const MidiMap* MidiData::getConstNotes() const
{
	return &data;
}

const std::vector<MidiNote*> MidiData::getOrderedConstNotes() const
{
	std::vector<MidiNote*> out_notes;
	out_notes.reserve(numNotes);

	for(auto mi : data)
	{
		if(mi.first == -1)
			continue;

		for(auto n : mi.second)
		{
			if(out_notes.size() < 1)
			{
				out_notes.push_back(n);
			}
			else
			{
				bool inserted = false;
				for(unsigned int i = 0; i < out_notes.size(); i++)
				{
					if(n->range.start > out_notes[i]->range.start)
					{
						out_notes.insert(out_notes.begin() + i, n);
						inserted = true;
						break;
					}
				}
				if(!inserted)
					out_notes.push_back(n);
			}
		}
	}

	return out_notes;
}

const std::vector<MidiNote*> MidiData::getOrderedConstNotesInRange(TimeRange range) const
{
	std::vector<MidiNote*> out_notes;

	for(auto mi : data)
	{
		if(mi.first < 0)
			continue;

		for(auto n : mi.second)
		{
			if(n->range.start <= range.end && range.start <= n->range.end)
			{
				if(out_notes.size() < 1)
				{
					out_notes.push_back(n);
				}
				else
				{
					bool inserted = false;
					for(unsigned int i = 0; i < out_notes.size(); i++)
					{
						if(n->range.start > out_notes[i]->range.start)
						{
							out_notes.insert(out_notes.begin() + i, n);
							inserted = true;
							break;
						}
					}
					if(!inserted)
						out_notes.push_back(n);
				}
			}
		}
	}

	return out_notes;
}
	
MidiSet MidiData::getNotesInRange(TimeRange range) const
{
	MidiSet out_notes;

	for(auto mi : data)
	{
		if(mi.first < 0)
			continue;

		for(auto n : mi.second)
		{
			if(n->range.start <= range.end && range.start <= n->range.end)
				out_notes.emplace(n);
		}
	}

	return out_notes;
}

//Parentheses return pointer to the note at the given midi index, and intersecting the given time.
//Returns null if no note matches the given criteria.
MidiNote* MidiData::operator()(MidiIndex mi, Time t)
{
	for(auto n : data[mi])
		if(t >= n->range.start && t < n->range.end)
			return n;

	return nullptr;
}
const MidiNote* MidiData::operator()(MidiIndex mi, Time t) const
{
	for(auto n : data.find(mi)->second)
		if(t >= n->range.start && t < n->range.end)
			return n;

	return nullptr;
}