#ifndef __SMDMIDI_HPP
#define __SMDMIDI_HPP
class smdMidi;
#include "midiFile.hpp"
#include "smdFile.hpp"

class smdMidi : public MidiFile {
	public:
		smdMidi&	AddToFile	(const smdSong &, int loops=0);
};

#endif
