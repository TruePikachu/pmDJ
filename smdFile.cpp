#include "smdFile.hpp"
#include <cstring>
#include <exception>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>
using namespace std;

std::ostream& operator<<(std::ostream& os, const smdSong &p) {
	os << "Name: " << p.name << "\nTrack count: " << p.tracks.size() << endl;
	os << "Instrument group: " << p.instrumentGroup << endl;
	for(vector< smdTrack >::const_iterator it=p.tracks.begin();it!=p.tracks.end();++it)
		os << *it << endl;
	return os;
}

smdSong::smdSong(std::ifstream& file) {
	off_t myStart = file.tellg();
	{ // Check magic numbers
		char magic[4];
		file.read(magic,sizeof(magic));
		if(strncmp(magic,"smdl",4))
			throw runtime_error("Not a .smd file");
		file.seekg(myStart+0x40);
		file.read(magic,sizeof(magic));
		if(strncmp(magic,"song",4))
			throw runtime_error("Doesn't look like a .smd file");
	}
	{ // Get name
		file.seekg(myStart+0x20);
		char fname[0x11];
		file.read(fname,0x10);
		fname[0x10]='\0';
		name = fname;
	}
	int nTrackChunks;
	{ // Get number of track chunks
		file.seekg(myStart+0x56);
		char count[1];
		file.read(count,1);
		nTrackChunks = count[0];
	}
	{ // Get instrument group
		file.seekg(myStart+0x0E);
		char field[1];
		file.read(field,1);
		instrumentGroup = field[0];
	}
	{ // Read the tracks
		file.seekg(myStart+0x80);
		for(int i=0;i<nTrackChunks;i++)
			tracks.push_back(smdTrack(file,instrumentGroup));
		// TODO try/catch block because of issue #1
	}
}

std::string smdSong::GetName() const {
	return name;
}

int smdSong::GetInstrumentGroup() const {
	return instrumentGroup;
}

int smdSong::GetTrackCount() const {
	return tracks.size();
}

const std::vector< smdTrack >& smdSong::Tracks() const {
	return tracks;
}

const smdTrack& smdSong::operator[](int i) const {
	return tracks[i];
}
bool smdSong::OutputInUse(int n) const {
	for(vector< smdTrack >::const_iterator it=tracks.begin();it!=tracks.end();++it)
		if(n==it->GetOutputID())
			return true;
	return false;
}

//////////

smdTrack::smdTrack(std::ifstream& file, int instrumentGroup) : instrumentGroup(instrumentGroup) {
	off_t myStart = file.tellg();
	{ // Check magic number
		char magic[4];
		file.read(magic,sizeof(magic));
		if(strncmp(magic,"trk ",4))
			throw runtime_error("Invalid SMD track header");
	}
	size_t dataLen;
	{ // Track data length
		file.seekg(myStart+0x0C);
		uint32_t rDataLen;
		file.read((char*)&rDataLen,4);
		dataLen = rDataLen;
	}
	{ // ID numbers
		char idNum[2];
		file.read(idNum,2);
		trackID = idNum[0];
		outputID = idNum[1];
	}
	{ // Track events
		file.seekg(myStart+0x14);
		smdEvent::EventType prevType;
		do {
			off_t startPos = file.tellg();
			try {
				if(startPos > myStart+0x10+dataLen)
					throw runtime_error("Read past end of track");
				smdEvent newEvent(file);
				prevType = newEvent.GetType();
				events.push_back(newEvent);
			} catch (exception& e) {
				stringstream errorMessage;
				char buffer[16];
				sprintf(buffer,"0x%08X",startPos);
				errorMessage << "smdTrack::smdTrack(): " << e.what() << " (trackID=" << trackID << ",startPos=" << buffer << ')';
				throw runtime_error(errorMessage.str());
			}
		} while (prevType != smdEvent::TRACK_END);
	}
	// Cue to end of track
	file.seekg(myStart+0x10+dataLen+((dataLen%4)?(4-(dataLen%4)):0));
}

std::ostream& operator<<(std::ostream& os, const smdTrack& p) {
	os << "tID=" << p.trackID << ", oID=" << p.outputID << endl;
	os << "Events: (" << p.events.size() << " count)\n";
	int when = 0;
	char whenBuffer[64];
	int loopPos = -1;
	for(int i=0;i<p.events.size();i++) {
		sprintf(whenBuffer,"%6u:%1u.%02u %-12u",when/192 +1,(when%192)/48 +1,(when%48),when);
		os << i << '\t' << whenBuffer << '\t' << p.events[i];
		if(p.events[i].GetType() == smdEvent::LOOP_POINT)
			loopPos = when;
		when += p.events[i].TickLength();
	}
	os << "(end events for tID=" << p.trackID;

	if(loopPos!=-1)
		os << ", LOOP=" << ((when-loopPos)/192) << ':' << ((when-loopPos)%192)/48 << '.' << ((when-loopPos)%48);
	os << ")\n";
	return os;
}

int smdTrack::GetTrackID() const {
	return trackID;
}

int smdTrack::GetOutputID() const {
	return outputID;
}

int smdTrack::GetInstrumentGroup() const {
	return instrumentGroup;
}

bool smdTrack::IsDrum() const {
	for(vector< smdEvent >::const_iterator it=events.begin();it!=events.end();++it)
		if(it->GetType() == smdEvent::SET_SAMPLE)
			if(it->params[0] >= 0x7C)
				return true;
	return false;
}

int smdTrack::GetEventCount() const {
	return events.size();
}

const std::vector< smdEvent >& smdTrack::Events() const {
	return events;
}

const smdEvent& smdTrack::operator[](int i) const {
	return events[i];
}

//////////

