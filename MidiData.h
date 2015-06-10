#ifndef APOLLO_MIDI_DATA_H
#define APOLLO_MIDI_DATA_H

#include "MIDI.h"

#include <array>
#include <unordered_set>

typedef std::unordered_map<MidiIndex, std::vector<MidiNote*>>	MidiMap;
typedef std::unordered_set<MidiNote*>							MidiSet;

class MidiData
{
protected:
	MidiMap			data;

	int				numNotes;
	TimeRange		span;
	Time			lengthOverride = -1.0;

	OwnedMutex		midiLock;

public:
	MidiData();
	MidiData(const std::vector<MidiNote> &initial_data);
	MidiData(const std::vector<MidiNote*> &initial_data);
	MidiData(const MidiData &other);
	virtual ~MidiData();

	MidiData& operator=(const MidiData &other);

	int getNumNotes() const;
	TimeRange getSpan() const;
	Time getLength() const;

	MidiNote* addNote(const MidiNote &note);
	std::vector<MidiNote*> addNotes(const std::vector<MidiNote> &new_notes);
	std::vector<MidiNote*> addNotes(const std::vector<MidiNote*> &new_notes);

	void setLength(Time new_length);

	//Returns true if successful.
	bool removeNote(MidiNote *note);
	void clearNotes();

	MidiMap* getNotes();
	const MidiMap* getConstNotes() const;
	const std::vector<MidiNote*> getOrderedConstNotes() const;
	const std::vector<MidiNote*> getOrderedConstNotesInRange(TimeRange range) const;
	
	MidiSet getNotesInRange(TimeRange range) const;
	//const MidiSet getConstNotesInRange(TimeRange range) const;

	//Parentheses return pointer reference to note at the given midi index, and intersecting the given time.
	MidiNote* operator()(MidiIndex mi, Time t);
	const MidiNote* operator()(MidiIndex mi, Time t) const;
};

#endif	//APOLLO_MIDI_DATA_H