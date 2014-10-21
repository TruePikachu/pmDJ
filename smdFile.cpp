#include "smdFile.hpp"
#include <cstring>
#include <exception>
#include <fstream>
#include <ostream>
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

//////////

