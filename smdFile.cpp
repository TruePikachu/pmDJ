#include "smdFile.hpp"
#include <cstdio>
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

bool smdSong::OutputInUseNotDrum(int n) const {
	for(vector< smdTrack >::const_iterator it=tracks.begin();it!=tracks.end();++it)
		if(n==it->GetOutputID())
			if(!it->IsDrum())
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
	smdEvent::DisplayBytes = p.LongestCmdSize();
	int when = 0;
	char whenBuffer[64];
	int loopPos = -1;
	for(int i=0;i<p.events.size();i++) {
		sprintf(whenBuffer,"%6u:%1u.%02u %-12u",when/192 +1,(when%192)/48 +1,(when%48),when);
		try {
			os << i << '\t' << whenBuffer << '\t' << p.events[i];
		} catch (exception& e) {
			stringstream errorMessage;
			char buffer[16];
			errorMessage << "operator<<(smdTrack): " << e.what() << " (trackID=" << p.trackID << " @ " << whenBuffer << ')';
			throw runtime_error(errorMessage.str());
		}
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

size_t smdTrack::LongestCmdSize() const {
	size_t result = 0;
	for(vector< smdEvent >::const_iterator it=events.begin();it!=events.end();++it)
		if(it->CmdSize()>result)
			result=it->CmdSize();
	return result;
}

//////////
int smdEvent::prevWaitLength;
size_t smdEvent::DisplayBytes;

smdEvent::smdEvent(std::ifstream& file) {
	char readByte;
	file.read(&readByte,1);
	eventCode = readByte;
	if(eventCode <= 0x7F) {
		// NOTE_PLAY
		file.read(&readByte,1);
		params.push_back(readByte);
		switch(params[0]&0xC0) {
			case 0x80:
				file.read(&readByte,1);
				params.push_back(readByte);
			case 0x40:
				file.read(&readByte,1);
				params.push_back(readByte);
			case 0x00:
				break;
			default:
				throw runtime_error("Bad flags");
		}
	} else if (eventCode > 0x8F)
		switch(eventCode) {
			case 0xDC:
				file.read(&readByte,1);
				params.push_back(readByte);
				file.read(&readByte,1);
				params.push_back(readByte);
			case 0xD4:
			case 0xE2:
			case 0xEA:
				file.read(&readByte,1);
				params.push_back(readByte);
			case WAIT_2BYTE:
			case 0xA8:
			case 0xB4:
			case 0xD6:
			case SET_BEND:
				file.read(&readByte,1);
				params.push_back(readByte);
			case WAIT_ADD:
			case WAIT_1BYTE:
			case 0x9C:
			case SET_OCTAVE:
			case SET_TEMPO:
			case 0xA9:
			case 0xAA:
			case SET_SAMPLE:
			case 0xB2:
			case 0xB5:
			case SET_MODU:
			case 0xBF:
			case 0xD0:
			case 0xD1:
			case 0xD2:
			case 0xDB:
			case SET_VOLUME:
			case SET_XPRESS:
			case SET_PAN:
			case 0xF4:
			case 0xF6:
				file.read(&readByte,1);
				params.push_back(readByte);
			case WAIT_AGAIN:
			case TRACK_END:
			case LOOP_POINT:
			case 0x9D:
			case 0xC0:
				break;
			default:
				{
					char buffer[8];
					sprintf(buffer,"UNK_%02X",eventCode);
				throw runtime_error((string)"Bad opcode: "+buffer);
				}
		}
	if((GetType()==WAIT_1BYTE) || (GetType()==WAIT_2BYTE))
		prevWaitLength = TickLength();
	if(GetType()==WAIT_ADD)
		prevWaitLength += (signed int8_t)params[0];
	waitAgainLength = prevWaitLength;
}

static const char* NoteNames[12] =
{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

std::ostream& operator<<(std::ostream& os, const smdEvent& p) {
	char buffer[200];
	sprintf(buffer,"0x%02X",p.eventCode);
	os << buffer;
	int howMany=smdEvent::DisplayBytes-1;
	if(howMany<=0)
		howMany=p.params.size();
	for(int i=0;i<howMany;i++)
		if(i>=p.params.size())
			os << "     ";
		else {
			sprintf(buffer," 0x%02X",p.params[i]);
			os << buffer;
		}
	os << '\t';

	if(p.eventCode <= 0x7F) {
		os << "Note: Velocity=" << (int)p.eventCode << ", ";
		switch((p.params[0]>>4) & 0x3) {
			case 0:
				os << "Oct-2, ";
				break;
			case 1:
				os << "Oct-1, ";
				break;
			case 3:
				os << "Oct+1, ";
				break;
		}
		if((p.params[0]&0xF) < 12)
			os << "Key=" << NoteNames[p.params[0]&0xF];
		else
			throw runtime_error("Bad key");
		switch(p.params[0]&0xC0) {
			case 0x00:
				break;
			case 0x40:
				os << ", Length=" << (int)p.params[1] << "/192";
				break;
			case 0x80:
				os << ", Length=" << (int)((p.params[1]<<8) + p.params[2]) << "/192";
		}
	} else
		switch(p.GetType()) {
			case smdEvent::DELTA_TIME:
				os << "Delta: ";
				os << "1/" << (2<<((p.eventCode&0xF)/3) + ((p.eventCode&0xF)%3 == 1));
				switch((p.eventCode&0xF)%3) {
					case 0:
						os << " note";
						break;
					case 1:
						os << " dot";
						break;
					case 2:
						os << " trip";
						break;
				}
				break;
			case smdEvent::WAIT_AGAIN:
				os << "WaitAgain: " << p.waitAgainLength << "/192";
				break;
			case smdEvent::WAIT_ADD:
				os << "WaitAdd: " << p.waitAgainLength << "/192 (d=" << (int)(int8_t)p.params[0] << "/192)";
				break;
			case smdEvent::WAIT_1BYTE:
				sprintf(buffer,"Wait: 0x%02X ( ",p.params[0]);
				os << buffer;
				os << (int)p.params[0] << "/192 )";
				break;
			case smdEvent::WAIT_2BYTE:
				sprintf(buffer,"Wait: 0x%02X%02X ( ",p.params[1],p.params[0]);
				os << buffer;
				os << (int)p.params[0] + (int)p.params[1]*256 << "/192 )";
				break;
			case smdEvent::TRACK_END:
				os << "-- END --";
				break;
			case smdEvent::LOOP_POINT:
				os << "-- LOOP --";
				break;
			case smdEvent::SET_OCTAVE:
				os << "Octave: " << (int)p.params[0];
				break;
			case smdEvent::SET_TEMPO:
				os << "Tempo: " << (int)p.params[0] << "BPM";
				break;
			case smdEvent::SET_SAMPLE:
				os << "Instrument: " << (int)p.params[0];
				break;
			case smdEvent::SET_MODU:
				os << "Modulation: " << (int)p.params[0];
				break;
			case smdEvent::SET_BEND:
				os << "Bend: " << (int16_t)(p.params[0]*256 + p.params[1]) << " cents";
				break;
			case smdEvent::SET_VOLUME:
				os << "Volume: " << (int)p.params[0];
				break;
			case smdEvent::SET_XPRESS:
				os << "Express: " << (int)p.params[0];
				break;
			case smdEvent::SET_PAN:
				os << "Pan: " << (int)(p.params[0] - 0x40);
				break;
			default:
				sprintf(buffer,"UNK_%2X",p.eventCode);
				os << buffer;
		}
	os << endl;
	return os;
}

smdEvent::EventType smdEvent::GetType() const {
	if(eventCode <= 0x7F)
		return NOTE_PLAY;
	if(eventCode <= 0x8F)
		return DELTA_TIME;
	switch(eventCode) {
		case WAIT_AGAIN:
		case WAIT_ADD:
		case WAIT_1BYTE:
		case WAIT_2BYTE:
		case TRACK_END:
		case LOOP_POINT:
		case SET_OCTAVE:
		case SET_TEMPO:
		case SET_SAMPLE:
		case SET_MODU:
		case SET_BEND:
		case SET_VOLUME:
		case SET_XPRESS:
		case SET_PAN:
			return (EventType)eventCode;
		default:
			return GENERAL_UNKNOWN;
	}
}

uint8_t smdEvent::GetEventCode() const {
	return eventCode;
}

int smdEvent::GetParamCount() const {
	return params.size();
}

uint8_t smdEvent::Param(int i) const {
	return params[i];
}

static const int DTimeTickTable[16] =
{96,72,64,48,36,32,24,18,16,12,9,8,6,4,3,2};

int smdEvent::TickLength() const {
	switch(GetType()) {
		case DELTA_TIME:
			return DTimeTickTable[eventCode-0x80];
		case WAIT_AGAIN:
		case WAIT_ADD:
			return waitAgainLength;
		case WAIT_1BYTE:
			return params[0];
		case WAIT_2BYTE:
			return params[0] + 256*params[1];
		default:
			return 0;
	}
}

size_t smdEvent::CmdSize() const {
	return 1+params.size();
}
