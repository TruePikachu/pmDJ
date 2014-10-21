#ifndef __SMDMIDI_HPP
#define __SMDMIDI_HPP
class smdMidi;
#include "midiFile.hpp"
#include "smdFile.hpp"

class smdMidi : public MidiFile {
	private:
		int	absoluteWriteTime;
	public:
				 smdMidi	();
		smdMidi&	AddToFile	(const smdSong &, int loops=0, int trim=0);
		// loops=0 means no loop, =1 means one loop (play loop part 2 times)
		// trim is the number of ticks to remove from the last loop
};

#endif
