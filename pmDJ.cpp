#include "instMap.hpp"
#include "midiFile.hpp"
#include "smdFile.hpp"
#include "smdMidi.hpp"
#include <cstdio>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

typedef struct {
	std::string	filename;
	int		loopCount;
	int		trimAmount;
} InFile;

int main(int argc, char *argv[]) {
	if(argc<3) {
		cout << "Usage: " << argv[0] << " <.smd file> [parameters] [<.smd file> [parameters] ...] <.mid file>\n";
		cout 	<< "Parameters can be:\n"
			<< "  -l# - Loop # times\n"
			<< "  -t#:#.# - Trim #:#.# time off end of final loop\n";
		return 0;
	}
	vector< InFile > inFiles;

	// Parse command line
	{
		for(int i=1;i<argc-1;i++)
			if(argv[i][0]=='-')
				// Probably an option
				switch(argv[i][1]) {
					case 'l':
					case 'L':
						{
							unsigned count;
							sscanf(argv[i]+2,"%u",&count);
							inFiles[inFiles.size()-1].loopCount = count;
						} break;
					case 't':
					case 'T':
						{
							unsigned measure,beat,tick;
							sscanf(argv[i]+2,"%u:%1u.%02u",&measure,&beat,&tick);
							inFiles[inFiles.size()-1].trimAmount = 192*measure + 48*beat + tick;
						}
						break;
					default:
						throw runtime_error((string)"Unknown parameter: "+argv[i]);
				}
			else {
				InFile myFile;
				myFile.filename = argv[i];
				myFile.loopCount=0;
				myFile.trimAmount=0;
				inFiles.push_back(myFile);
			}		
	}

	return 0;
}
