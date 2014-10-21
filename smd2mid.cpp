#include "midiFile.hpp"
#include "smdFile.hpp"
#include "smdMidi.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <string>
using namespace std;

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

	// Process file
	smdMidi myMidi;
	myMidi.AddToFile(mySong,loops-1);

	// Write file
	ofstream midFile(argv[2]);
	myMidi.Save(midFile);
	midFile.close();
	return 0;
}
