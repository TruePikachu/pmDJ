#ifndef __MIDIFILE_HPP
#define __MIDIFILE_HPP
class MidiFile;
class MidiTrack;
class MidiEvent;
#include <fstream>
#include <stdint.h>
#include <vector>

class MidiFile {
	private:
		std::vector< MidiTrack > tracks;
	public:
					 MidiFile();
					 MidiFile(std::ifstream&);
		// Member functions
		std::vector< MidiTrack >&	Tracks	();
		const std::vector< MidiTrack >&	Tracks	() const;
		// File I/O
		const MidiFile&		Save	(std::ofstream&) const;
};

class MidiTrack {
	private:
		friend class MidiFile;
		std::vector< MidiEvent >	events;
		int				padToTime;
		const MidiTrack&Write	(std::ofstream&) const;
		MidiTrack&	Read	(std::ifstream&);
	public:
				 MidiTrack	();
		// Member functions
		std::vector< MidiEvent >&	Events	();
		const std::vector< MidiEvent >&	Events	() const;
		MidiTrack&	AddEvent	(MidiEvent);
		int		GetPadTime	() const;
		MidiTrack&	SetPadTime	(int);
		MidiTrack&	StopNotes	(int when);
};

class MidiEvent {
	public:
		typedef enum {
			NOTE_ON,
			NOTE_OFF,
			CONTROLLER,
			PROGRAM,
			BEND,
			META
		} EventType;
	private:
		friend class MidiTrack;
		static int	wroteTime;	// Used for reading/writing
		static long int	makeIndex;	// Used for sorting events
		long int	madeIndex;	//   in create order
		int		absoluteTime;
		EventType	eventType;
		uint8_t		channel;
		uint8_t		param1;	// NOTE key, CONTROLLER number,
					// PROGRAM id, BEND MSB, META type
		uint8_t		param2;	// NOTE vel, CONTROLLER value,
					// BEND LSB
		size_t		metaLength;
		char*		metaData;
		const MidiEvent&Write	(std::ofstream&) const;
				 MidiEvent(std::ifstream&);
	public:
				 MidiEvent	(int,EventType,uint8_t,uint8_t,uint8_t);	// NOTE and CONTROLLER
				 MidiEvent	(int,EventType,uint8_t,uint16_t);		// PROGRAM and BEND
				 MidiEvent	(int,uint8_t,size_t,const char*);		// META
				 MidiEvent	(const MidiEvent&);
				~MidiEvent	();
		MidiEvent&	operator=	(MidiEvent);

		EventType	GetType		() const;
		int		AbsoluteTime	() const;
		bool		operator<	(const MidiEvent&) const;
};

#endif
