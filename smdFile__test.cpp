#include "smdFile.hpp"
#include <cstdlib>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <tap++/tap++.h>
#include <vector>
using namespace std;
using namespace TAP;

int main() {
	plan(2);
	smdSong *myFile;
	try {
		ifstream myFHandle(PKGDIR "/test_data/bgm0020.smd");
		myFile = new smdSong(myFHandle);
		myFHandle.close();
	} catch (exception& e) {
		diag("Exception: ", e.what());
		ok(false,"Read SMD");
		skip(1);
		return exit_status();
	}
	ok(true,"Read SMD");
	try {
		stringstream myStream;
		myStream << myFile;
	} catch (exception &e) {
		diag("Exception: ", e.what());
		ok(false,"Dump SMD");
		return exit_status();
	}
	ok(true,"Dump SMD");
	delete myFile;
	return exit_status();
}
