#ifndef APOLLO_NOTE_H
#define APOLLO_NOTE_H

#include "MIDI.h"

//Represents a note value on the piano (e.g. C4 = middle C, G4s = A4f = G4-sharp = A4=flat)
//	The value of each Note is the corresponding MidiIndex
enum class NoteValue : MidiIndex
{
	INVALID = -2,
	REST = -1,

	A0 = 21,
	A0s = 22, B0f = 22,
	B0 = 23,
	C1 = 24,
	C1s = 25, D1f = 25,
	D1 = 26,
	D1s = 27, E1f = 27,
	E1 = 28,
	F1 = 29,
	F1s = 30, G1f = 30,
	G1 = 31,
	G1s = 32, A1f = 32,
	
	A1 = 33,
	A1s = 34, B1f = 34,
	B1 = 35,
	C2 = 36,
	C2s = 37, D2f = 37,
	D2 = 38,
	D2s = 39, E2f = 39,
	E2 = 40,
	F2 = 41,
	F2s = 42, G2f = 42,
	G2 = 43,
	G2s = 44, A2f = 44,
	
	A2 = 45,
	A2s = 46, B2f = 46,
	B2 = 47,
	C3 = 48,
	C3s = 49, D3f = 49,
	D3 = 50,
	D3s = 51, E3f = 51,
	E3 = 52,
	F3 = 53,
	F3s = 54, G3f = 54,
	G3 = 55,
	G3s = 56, A3f = 56,
	
	A3 = 57,
	A3s = 58, B3f = 58,
	B3 = 59,
	C4 = 60,
	C4s = 61, D4f = 61,
	D4 = 62,
	D4s = 63, E4f = 63,
	E4 = 64,
	F4 = 65,
	F4s = 66, G4f = 66,
	G4 = 67,
	G4s = 68, A4f = 68,
	
	A4 = 69,
	A4s = 70, B4f = 70,
	B4 = 71,
	C5 = 72,
	C5s = 73, D5f = 73,
	D5 = 74,
	D5s = 75, E5f = 75,
	E5 = 76,
	F5 = 77,
	F5s = 78, G5f = 78,
	G5 = 79,
	G5s = 80, A5f = 80,
	
	A5 = 81,
	A5s = 82, B5f = 82,
	B5 = 83,
	C6 = 84,
	C6s = 85, D6f = 85,
	D6 = 86,
	D6s = 87, E6f = 87,
	E6 = 88,
	F6 = 89,
	F6s = 90, G6f = 90,
	G6 = 91,
	G6s = 92, A6f = 92,
	
	A6 = 93,
	A6s = 94, B6f = 94,
	B6 = 95,
	C7 = 96,
	C7s = 97, D7f = 97,
	D7 = 98,
	D7s = 99, E7f = 99,
	E7 = 100,
	F7 = 101,
	F7s = 102, G7f = 102,
	G7 = 103,
	G7s = 104, A7f = 104,

	A7 = 105,
	A7s = 106, B7f = 106,
	B7 = 107,
	C8 = 108
};
typedef NoteValue Notes;

//Returns the underlying MidiIndex of the Note
extern MidiIndex getIndex(const NoteValue &n);



//Represents a note
struct Note
{
	NoteValue	value = Notes::REST;	//Which note
	int			vel = 0;				//Velocity of note
	double		length;					//Length of note (in number of quarter notes)

	//For a rest
	Note(double note_length);

	//For a note
	Note(NoteValue note_value, int note_vel, double note_length);
};


//Represents a note or chord in a song
struct SongNote
{
	std::vector<Note>	notes;		//Notes that occur simultaneously
	double				offset;		//Offset to next SongNote in the song (in number of quarter notes)
	bool				staccato;	//Whether the note(s) is/are staccato (shortened)

	//For a single note
	SongNote(Note note, double offset_to_next, bool staccato_ = false);
	SongNote(NoteValue note_value, int note_vel, double note_length, double offset_to_next, bool staccato_ = false);

	//For chord
	SongNote(std::vector<Note> notes_, double offset_to_next, bool staccato_ = false);

};

//Represents a song --> can convert itself to MidiData
struct Song
{
	static const int DEFAULT_MIDDLE_VEL;

	std::vector<SongNote>	treble,	//Notes on top staff
							bass;	//Notes on bottom staff
	double					bpm;	//Default beats per minute

	unsigned int			numTreble,	//Number of treble notes
							numBass;	//Number of bass notes

	Song(double beats_per_minute, const std::vector<SongNote> &treble_notes, const std::vector<SongNote> &bass_notes);

	//Converts Song into MidiData --> if BMP <= 0, uses default bpm.
	void interpret(MidiData &data, double start_time, int middle_vel, double BPM = -1.0) const;
};

/////////////////////
/////// SONGS ///////
/////////////////////

//Legend of Zelda Main Theme Song
extern const Song	legendOfZeldaTheme1,
					legendOfZeldaTheme2;


#endif	//APOLLO_NOTE_H