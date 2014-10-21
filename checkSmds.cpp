#include "smdFile.hpp"
#include <errno.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
	if(argc==1) {
		cout << "Usage: " << argv[0] << " <List of .smd files>\n";
		return 0;
	}
	for(int n=1;n<argc;n++) {
		ifstream smdFile(argv[n]);
		try {
			smdSong smdSong(smdFile);
		} catch (exception& e) {
			cerr << "Error from " << argv[n] << ": " << e.what() << endl;
		}
	}
	return 0;
}
