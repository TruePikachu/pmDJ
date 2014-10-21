#include "midiFile.hpp"
#include <cstdlib>
#include <exception>
#include <fstream>
#include <string>
#include <tap++/tap++.h>
#include <vector>
using namespace std;
using namespace TAP;

int main() {
	plan(5);
	MidiFile myFile;
	try {
		MidiTrack myTrack1;
		myTrack1.AddEvent(MidiEvent(  0,0x51,3,"\x05\x99\x20"));
		myTrack1.AddEvent(MidiEvent(  0,MidiEvent::PROGRAM,0,0));
		myTrack1.AddEvent(MidiEvent(  0,MidiEvent::CONTROLLER,0,0x07,110));
		myTrack1.AddEvent(MidiEvent(  0,MidiEvent::CONTROLLER,0,0x0B,127));
		myTrack1.AddEvent(MidiEvent(  0,MidiEvent::NOTE_ON ,0,65,0x7F));
		myTrack1.AddEvent(MidiEvent( 24,MidiEvent::NOTE_OFF,0,65,0x7F));
		myTrack1.AddEvent(MidiEvent( 48,MidiEvent::NOTE_ON ,0,65,0x7F));
		myTrack1.AddEvent(MidiEvent( 60,MidiEvent::NOTE_OFF,0,65,0x7F));
		myTrack1.AddEvent(MidiEvent( 60,MidiEvent::NOTE_ON ,0,67,0x7F));
		myTrack1.AddEvent(MidiEvent( 72,MidiEvent::NOTE_OFF,0,67,0x7F));
		myTrack1.AddEvent(MidiEvent( 72,MidiEvent::NOTE_ON ,0,69,0x7F));
		myTrack1.AddEvent(MidiEvent( 96,MidiEvent::NOTE_OFF,0,69,0x7F));
		myTrack1.AddEvent(MidiEvent(120,MidiEvent::NOTE_ON ,0,67,0x7F));
		myTrack1.AddEvent(MidiEvent(144,MidiEvent::NOTE_OFF,0,67,0x7F));
		myTrack1.AddEvent(MidiEvent(144,MidiEvent::NOTE_ON ,0,65,0x7F));
		myTrack1.AddEvent(MidiEvent(168,MidiEvent::NOTE_OFF,0,65,0x7F));
		myTrack1.AddEvent(MidiEvent(576,MidiEvent::NOTE_ON ,0,72,0x7F));
		myTrack1.AddEvent(MidiEvent(600,MidiEvent::NOTE_OFF,0,72,0x7F));
		myTrack1.AddEvent(MidiEvent(624,MidiEvent::NOTE_ON ,0,72,0x7F));
		myTrack1.AddEvent(MidiEvent(636,MidiEvent::NOTE_OFF,0,72,0x7F));
		myTrack1.AddEvent(MidiEvent(636,MidiEvent::NOTE_ON ,0,73,0x7F));
		myTrack1.AddEvent(MidiEvent(648,MidiEvent::NOTE_OFF,0,73,0x7F));
		myTrack1.AddEvent(MidiEvent(648,MidiEvent::NOTE_ON ,0,72,0x7F));
		myTrack1.AddEvent(MidiEvent(672,MidiEvent::NOTE_OFF,0,72,0x7F));
		myTrack1.AddEvent(MidiEvent(696,MidiEvent::NOTE_ON ,0,71,0x7F));
		myTrack1.AddEvent(MidiEvent(720,MidiEvent::NOTE_OFF,0,71,0x7F));
		myTrack1.AddEvent(MidiEvent(720,MidiEvent::NOTE_ON ,0,68,0x7F));
		myTrack1.AddEvent(MidiEvent(744,MidiEvent::NOTE_OFF,0,68,0x7F));
		myFile.Tracks().push_back(myTrack1);
		MidiTrack myTrack2;
		myTrack2.AddEvent(MidiEvent(  0,MidiEvent::PROGRAM,1,10));
		myTrack2.AddEvent(MidiEvent(  0,MidiEvent::CONTROLLER,1,0x07,110));
		myTrack2.AddEvent(MidiEvent(  0,MidiEvent::CONTROLLER,1,0x0B,127));
		myTrack2.AddEvent(MidiEvent(216+  0,MidiEvent::NOTE_ON ,1,72,0x7F));
		myTrack2.AddEvent(MidiEvent(216+144,MidiEvent::NOTE_OFF,1,72,0x7F));
		myTrack2.AddEvent(MidiEvent(288+  0,MidiEvent::NOTE_ON ,1,69,0x7F));
		myTrack2.AddEvent(MidiEvent(288+144,MidiEvent::NOTE_OFF,1,69,0x7F));
		myTrack2.AddEvent(MidiEvent(360+  0,MidiEvent::NOTE_ON ,1,77,0x7F));
		myTrack2.AddEvent(MidiEvent(360+144,MidiEvent::NOTE_OFF,1,77,0x7F));
		myTrack2.AddEvent(MidiEvent(432+  0,MidiEvent::NOTE_ON ,1,72,0x7F));
		myTrack2.AddEvent(MidiEvent(432+144,MidiEvent::NOTE_OFF,1,72,0x7F));
		myTrack2.AddEvent(MidiEvent(864+  0,MidiEvent::NOTE_ON ,1,67,0x7F));
		myTrack2.AddEvent(MidiEvent(864+144,MidiEvent::NOTE_OFF,1,67,0x7F));
		myTrack2.AddEvent(MidiEvent(936+  0,MidiEvent::NOTE_ON ,1,72,0x7F));
		myTrack2.AddEvent(MidiEvent(936+144,MidiEvent::NOTE_OFF,1,72,0x7F));
		myTrack2.AddEvent(MidiEvent(1008+ 0,MidiEvent::NOTE_ON ,1,65,0x7F));
		myTrack2.AddEvent(MidiEvent(1008+144,MidiEvent::NOTE_OFF,1,65,0x7F));
		myFile.Tracks().push_back(myTrack2);
	} catch (exception& e) {
		diag("Exception: ", e.what());
		ok(false,"Create data");
		skip(4);
		return exit_status();
	}
	ok(true,"Create data");
	try {
		ofstream myFileMid(".test-midi.mid");
		myFile.Save(myFileMid);
		myFileMid.close();
	} catch (exception& e) {
		diag("Exception: ", e.what());
		ok(false,"Save data");
		skip(3);
		return exit_status();
	}
	ok(true,"Save data");
	is(system("diff -q .test-midi.mid " PKGDIR "/test_data/totaka.mid"),0,"Diff data");

	MidiFile myOtherFile;
	try {
		ifstream myFileMid(".test-midi.mid");
		myOtherFile.Load(myFileMid);
		myFileMid.close();
	} catch (exception& e) {
		diag("Exception: ", e.what());
		ok(false,"Load data");
		skip(1);
		return exit_status();
	}
	ok(true,"Load data");

	try {
		ofstream myFileMid(".test-midi.mid");
		myOtherFile.Save(myFileMid);
		myFileMid.close();
	} catch (exception& e) {
		diag("Exception: ", e.what());
		ok(false,"Rediff data");
		return exit_status();
	}
	is(system("diff -q .test-midi.mid " PKGDIR "/test_data/totaka.mid"),0,"Rediff data");
	system("rm .test-midi.mid");
	return exit_status();
}
