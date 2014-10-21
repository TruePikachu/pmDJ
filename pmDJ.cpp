#include "instMap.hpp"
#include "midiFile.hpp"
#include "smdFile.hpp"
#include "smdMidi.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main(int argc, char *argv[]) {
	if(argc<3) {
		cout << "Usage: " << argv[0] << " <.smd file> [parameters] [<.smd file> [parameters] ...] <.mid file>\n";
		cout 	<< "Parameters can be:\n"
			<< "  -l# - Loop # times\n"
			<< "  -t#:#.# - Trim #:#.# time off end of final loop\n";
		return 0;
	}
	return 0;
}
