#include "midiFile.hpp"
#include "smdRead.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <string>
using namespace std;

ofstream& WriteByte(ofstream&,uint8_t);
ofstream& WriteWord(ofstream&,uint16_t);
ofstream& WriteDWord(ofstream&,uint32_t);
ofstream& WriteLong(ofstream&,long unsigned);



int main(int argc, char *argv[]) {
	if(argc < 3 || argc > 4) {
		cout << "Usage: " << argv[0] << " <.smd file> <.mid file> [loops]\n";
		return 0;
	}

	// Read file
	ifstream smdFile(argv[1]);
	smdSong mySong(smdFile);

	unsigned loops = 1;
	if(argc==4)
		sscanf(argv[3],"%u",&loops);

	// Write file
	ofstream midFile(argv[2]);
	MidiFile myMidi;
	myMidi.LoadSMD(mySong,loops);
	if (myMidi.WriteFile(midFile))
		return 0;
	return 1;
}
