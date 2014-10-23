#include "swdFile.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
	if(argc!=2) {
		cout << "Usage: " << argv[0] << " <.swd file>\n";
		return 0;
	}
	ifstream myFile(argv[1]);
	swdFile mySwd(myFile);
	myFile.close();
	cout << mySwd << endl;
	return 0;
}
