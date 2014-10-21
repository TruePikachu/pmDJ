#include "smdMidi.hpp"
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
	plan(3);
	smdMidi mySM;
	try {
		ifstream mySH(PKGDIR "/test_data/bgm0020.smd");
		smdSong mySmd(mySH);
		mySH.close();
		mySM.AddToFile(mySmd,0);
	} catch (exception& e) {
		diag("Exception: ",e.what());
		ok(false,"AddToFile()");
		skip(2);
		return exit_status();
	}
	ok(true,"AddToFile()");
	try {
		ofstream myMH(".test-midi.mid");
		mySM.Save(myMH);
		myMH.close();
	} catch (exception& e) {
		diag("Exception: ",e.what());
		ok(false,"MidiFile::Save()");
		skip(1);
		return exit_status();
	}
	ok(true,"MidiFile::Save()");
	is(system("diff -q .test-midi.mid " PKGDIR "/test_data/bgm0020.mid"),0,"Diff data");
	system("rm .test-midi.mid");
	return exit_status();
}
