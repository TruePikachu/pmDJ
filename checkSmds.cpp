#include "smdRead.hpp"
#include <errno.h>
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
			smdSong smdSong(smdFile,true);
		} catch (int error) {
			cerr << "Error from " << argv[n] << endl;
			if(error != EBADRQC)
				return 1;
		}
	}
	return 0;
}
