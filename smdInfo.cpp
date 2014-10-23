#include "smdFile.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
	char buffer[256];
	if(argc==1) {
		cout << "Usage: " << argv[0] << " <smd files>\n";
		return 0;
	}
	for(int i=1;i<argc;i++) {
		ifstream myFile(argv[i]);
		smdSong mySong(myFile);
		myFile.close();

		cout << argv[i] << " (" << mySong.GetName() << "):\n";
		cout << mySong.GetTrackCount() << " tracks, instrument group " << mySong.GetInstrumentGroup() << endl;
		for(vector< smdTrack >::const_iterator trk=mySong.Tracks().begin();trk!=mySong.Tracks().end();++trk) {
			int loopPos=-1;
			int readPos=0;
			for(vector< smdEvent >::const_iterator evt=trk->Events().begin();evt!=trk->Events().end();++evt)
				switch(evt->GetType()) {
					case smdEvent::LOOP_POINT:
						loopPos=readPos;
					case smdEvent::TRACK_END:
						break;
					default:
						readPos+=evt->TickLength();
				}
			cout << ((trk->GetTrackID()<10)?"t ":"t") << trk->GetTrackID() << ((trk->GetOutputID()<10)?" o ":" o") << trk->GetOutputID() << " " << (trk->IsDrum()?'d':' ') << "  Length:";
			sprintf(buffer,"%6u:%1u.%02u=%-12u",readPos/192,(readPos%192)/48,readPos%48,readPos);
			cout << buffer;
			if(loopPos!=-1) {
				sprintf(buffer,"%6u:%1u.%02u=%-12u",loopPos/192 +1,(loopPos%192)/48 +1, loopPos%48,loopPos);
				cout << "  Loop@" << buffer;
				sprintf(buffer,"%6u:%1u.%02u=%-12u",(readPos-loopPos)/192,((readPos-loopPos)%192)/48,(readPos-loopPos)%48,readPos-loopPos);
				cout << " (length:" << buffer << ")";
			}
			cout << endl;
		}
	}
	return 0;
}
