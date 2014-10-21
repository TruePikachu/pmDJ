#include "smdRead.hpp"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
	if(argc != 2) {
		cout << "Usage: " << argv[0] << " <.smd file>\n";
		return 0;
	}
	
	ifstream theFile(argv[1]);
	smdSong mySong(theFile);
	cout << mySong << endl;
	return 0;
}
