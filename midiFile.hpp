#ifndef __MIDIFILE_HPP
#define __MIDIFILE_HPP
class MidiFile;
class MidiTrack;
class MidiEvent;
#include "smdRead.hpp"
#include <fstream>
#include <stdint.h>
#include <vector>
std::ofstream& WriteByte(std::ofstream&, uint8_t);
std::ofstream& WriteWord(std::ofstream&, uint16_t);
std::ofstream& WriteDWord(std::ofstream&, uint32_t);
std::ofstream& WriteLong(std::ofstream&, unsigned long);
size_t EncodeVarLen(unsigned long source, size_t destByteCount, char *pData);
// returns minimum number of bytes for the value

class MidiFile {
	private:
		friend std::ostream& operator<<(std::ostream&, const MidiFile&);
	public:
		std::vector< MidiTrack >	tracks;
		MidiFile&	LoadSMD		(const smdSong&, unsigned loops=1);
		bool		WriteFile	(std::ofstream&) const;
		MidiFile&	ApplyRemap	(const uint8_t[256]);
};

class MidiTrack {
	private:
		friend class MidiFile;
		bool	WriteFile	(std::ofstream&) const;
		friend std::ostream& operator<<(std::ostream&, const MidiTrack&);
	public:
		std::vector< MidiEvent >	events;

				 MidiTrack	(const smdTrack &, unsigned loops=1, int freeOut = 9);
		MidiTrack&	ApplyRemap	(const uint8_t[256]);
};

class MidiEvent {
	public:
		typedef enum {
				NOTE_ON,
				NOTE_OFF,
				CONTROLLER,
				PROGRAM,
				META_EVENT,
				SET_RPN,
				BEND
			} EventType;
	private:
		static long int makeIndex;
		static int lastSentRPN;
		long int madeIndex;
		friend class MidiTrack;
		int		absoluteTime;
		EventType	eventType;
		uint8_t		channel;
		uint8_t		param1;	// or META type, or RPN id, or bend LSB
		uint8_t		param2; // or bend MSB
		size_t		metaLength;
		char*		metaData;
		uint16_t	rpnData;

		bool		WriteFile	(std::ofstream&,int) const;
		friend std::ostream& operator<<(std::ostream&, const MidiEvent&);

		MidiEvent&	RemapInstrument	(const uint8_t[256]);

	public:
			 MidiEvent(int,EventType,uint8_t,uint8_t,uint8_t);
			 MidiEvent(int,uint8_t,size_t,const char*);	// META
			 MidiEvent(int,uint8_t,int,uint16_t);	// RPN
			 MidiEvent(const MidiEvent&);
			~MidiEvent();
		MidiEvent& operator=(MidiEvent);

		// For sorting based on time
		bool 		operator>	(const MidiEvent&) const;
		bool		operator<	(const MidiEvent&) const;
		bool		operator>=	(const MidiEvent&) const;
		bool		operator<=	(const MidiEvent&) const;
		bool		operator==	(const MidiEvent&) const;
};

#endif
