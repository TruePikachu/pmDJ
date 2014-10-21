#include "smdRead.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
int smdEvent::prevWaitLength = -1;

std::ostream& operator<<(std::ostream& os, const smdSong& p) {
	os << "Name: " << p.name << "\nTrack count: " << p.trackCount << endl;
	os << "UNK_AA field: " << (int)p.aaField << endl;
	for(int i=0;i<17;i++)
		if(p.tracks[i])
			os << "Track " << i << ": " << *p.tracks[i] << endl;
	return os;
}

smdSong::smdSong(std::ifstream& file, bool throwToCaller) {
	for(int i=0;i<17;i++)
		tracks[i] = 0;
	off_t myStart = file.tellg();
	// Check magic numbers
	{
//		file.seekg(myStart+0x00);
		char magic[4];
		file.read(magic,sizeof(magic));
		if(strncmp(magic,"smdl",sizeof(magic)))
			throw EINVAL;

		file.seekg(myStart+0x40);
		file.read(magic,sizeof(magic));
		if(strncmp(magic,"song",sizeof(magic)))
			throw EINVAL;
	}
	// Get name
	{
		file.seekg(myStart+0x20);
		char fname[0x10+1];
		file.read(fname,sizeof(fname));
		fname[sizeof(fname)-1]='\0';
		name = fname;
	}
	// Get the number of track chunks
	{
		file.seekg(myStart+0x56);
		char count[1];
		file.read(count,sizeof(count));
		trackCount = count[0];
	}
	// Get AA field
	{
		file.seekg(myStart+0x0E);
		char field[1];
		file.read(field,1);
		aaField = field[0];
	}
	// Read the tracks
	{
		file.seekg(myStart+0x80);
		for(int i=0;i<trackCount;i++) {
			smdTrack *newTrack = new smdTrack(file,throwToCaller,aaField);
			tracks[newTrack->GetID()] = newTrack;
		}
	}
}

smdSong::~smdSong() {
	for(int i=0;i<17;i++)
		if(tracks[i])
			delete tracks[i];
}

const smdTrack* smdSong::operator[](int i) const {
	return tracks[i];
}

smdTrack::smdTrack(std::ifstream& file,bool throwToCaller,int aaField) : aaField(aaField) {
	off_t myStart = file.tellg();
	// Check magic number
	{
//		file.seekg(myStart+0x00);
		char magic[4];
		file.read(magic,sizeof(magic));
		if(strncmp(magic,"trk ",sizeof(magic)))
			throw EINVAL;
	}
	// Get track data length
	size_t dataLen;
	{
		file.seekg(myStart+0x0C);
		uint32_t rDataLen;
		file.read((char*)&rDataLen,sizeof(rDataLen));
		dataLen = rDataLen;
	}
	// Get ID numbers
	{
//		file.seekg(myStart+0x10);
		char idNum[2];
		file.read(idNum,sizeof(idNum));
		trackID = idNum[0];
		outputID = idNum[1];
	}
	// Read track events
	{
		file.seekg(myStart+0x14);
		smdEvent::EVENT_TYPE prevType;
		do {
			off_t startPos = file.tellg();
			try {
				if(startPos > myStart+0x10+dataLen)
					throw EFAULT;
				smdEvent newEvent(file);
				prevType = newEvent.GetType();
				events.push_back(newEvent);
			} catch (int error) {
				char buf[32];
				sprintf(buf,"0x%08X",startPos);
				cerr << "smdTrack::smdTrack(): " << strerror(error) << endl;
				cerr << "  trackID=" << trackID << endl;
				cerr << "  events.size()=" << events.size() << endl;
				cerr << "  startPos=" << buf << endl;
				cerr << "  Bytes: ";
				file.seekg(startPos);
				for(int rP=0;rP<10;rP++) {
					uint8_t c;
					file.read((char*)&c,1);
					sprintf(buf," 0x%02X",c);
					cerr << buf;
				}
				cerr << endl;

				if(throwToCaller)
					throw error;
				break;
			}
		} while (prevType != smdEvent::TRACK_END);
	}
	// Cue to end of track
	file.seekg(myStart+0x10+dataLen+((dataLen%4)?(4-(dataLen%4)):0));
}

const std::vector< smdEvent >& smdTrack::GetEvents() const {
	return events;
}

smdEvent::smdEvent(std::ifstream& file) {
	usedParam1=false;
	usedParam2=false;
	usedParam3=false;
	char readByte;
	file.read(&readByte,1);
	eventCode = readByte;
	if(eventCode<=0x7F) {
		file.read(&readByte,1);
		param1=readByte; usedParam1=true;
		switch(param1 & 0xC0) {
			case 0x40:
				file.read(&readByte,1);
				param2=readByte;
				usedParam2=true;
				break;
			case 0x80:
				file.read(&readByte,1);
				param2=readByte;
				usedParam2=true;
				file.read(&readByte,1);
				param3=readByte;
				usedParam3=true;
				break;
		}
	} else if (eventCode > 0x8F)
		switch(eventCode) {
			case WAIT_AGAIN:
			case TRACK_END:
			case LOOP_POINT:
			case 0xC0:
				break;
			case WAIT_ADD:
			case WAIT_1BYTE:
			case SET_OCTAVE:
			case SET_TEMPO:
			case 0xA9:
			case 0xAA:
			case SET_SAMPLE:
			case SET_MODU:
			case 0xBF:	// ???
			case 0xDB:
			case SET_VOLUME:
			case SET_XPRESS:
			case SET_PAN:
			case 0xF4:	// ???
			case 0xF6:	// ???
				file.read(&readByte,1);
				param1=readByte; usedParam1=true;
				break;
			case WAIT_2BYTE:
			case 0xD6:	// ???
			case SET_BEND:
				file.read(&readByte,1);
				param1=readByte; usedParam1=true;
				file.read(&readByte,1);
				param2=readByte; usedParam2=true;
				break;
			case 0xA8:
			case 0xB2:
			case 0xE2:
				throw EBADRQC;
			default:
				throw ENOTSUP;
		}
	if((GetType()==WAIT_1BYTE) || (GetType()==WAIT_2BYTE))
		prevWaitLength = TickLength();
	if(GetType()==WAIT_ADD)
		prevWaitLength += (signed int8_t)param1;
	waitAgainLength = prevWaitLength;
}

std::ostream& operator<<(std::ostream& os, const smdTrack& p) {
	os << "tID=" << p.trackID << ", oID=" << p.outputID +1 << endl;
	os << "Events: (" << p.events.size() << " count)\n";
	int when = 0;
	char whenBuffer[64];
	int loopPos = -1;
	for(int i=0;i<p.events.size();i++) {
		sprintf(whenBuffer,"%6u:%1u.%02u %-12u",when/192 +1,(when%192)/48 +1,(when%48),when);
		os << i << '\t' << whenBuffer << '\t' << p.events[i];
		if(p.events[i].GetType() == smdEvent::LOOP_POINT)
			loopPos=when;
		when += p.events[i].TickLength();
	}
	os << "(end events for tID=" << p.trackID;
	
	if (loopPos!=-1)
		os << ", LOOP=" << ((when-loopPos)/192) << ':' << ((when-loopPos)%192)/48 << '.' << ((when-loopPos)%48);
	os << ")\n";
	return os;
}

int smdTrack::GetID() const {
	return trackID;
}

int smdTrack::GetOutput() const {
	return outputID;
}

std::ostream& operator<<(std::ostream& os, const smdEvent& p) {
	char buffer[200];
	sprintf(buffer,"0x%02X",p.eventCode);
	os << buffer;
	if(p.usedParam1) {
		sprintf(buffer," 0x%02X",p.param1);
		os << buffer;
	} else
		os << "     ";
	if(p.usedParam2) {
		sprintf(buffer," 0x%02X",p.param2);
		os << buffer;
	} else
		os << "     ";
	if(p.usedParam3) {
		sprintf(buffer," 0x%02X",p.param3);
		os << buffer;
	} else
		os << "     ";
	os << "\t";
	if(p.eventCode <= 0x7F) {
		os << "Note: Velocity=" << (int)p.eventCode << ", param1=" << (int)p.param1;
		sprintf(buffer," (flag=%X (",p.param1>>4);
		os << buffer;
		switch((p.param1>>4) & 0x3) {
			case 0:
				os << "Oct-2";
				break;
			case 1:
				os << "Oct--";
				break;
			case 2:
				os << "Oct==";
				break;
			case 3:
				os << "Oct++";
				break;
		}
		switch((p.param1>>4) & 0xC) {
			case 0:
				break;
			case 4:
				os << " LEN";
				break;
			case 8:
				os << "LEN2";
				break;
			case 0xC:
				os << "???";
				break;
		}
		os << "), key=";
		switch(p.param1&0xF) {
			case 0x0:
				os << "C";
				break;
			case 0x1:
				os << "C#";
				break;
			case 0x2:
				os << "D";
				break;
			case 0x3:
				os << "D#";
				break;
			case 0x4:
				os << "E";
				break;
			case 0x5:
				os << "F";
				break;
			case 0x6:
				os << "F#";
				break;
			case 0x7:
				os << "G";
				break;
			case 0x8:
				os << "G#";
				break;
			case 0x9:
				os << "A";
				break;
			case 0xA:
				os << "A#";
				break;
			case 0xB:
				os << "B";
				break;
			default:
				throw EINVAL;
		}
		os << ')';
		switch(p.param1 & 0xC0) {
			case 0x00:
				break;
			case 0x40:
				os << ", param2=" << (int)p.param2 << "/192 length";
				break;
			case 0x80:
				os << ", param2+3=" << (int)p.param3 + (int)p.param2*256 << "/192 length";
				break;
			default:
				os << "??? PARAM";
				break;
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
				}
				break;
			case smdEvent::WAIT_AGAIN:
				os << "WaitAgain: " << p.waitAgainLength << "/192";
				break;
			case smdEvent::WAIT_ADD:
				os << "WaitAdd: " << p.waitAgainLength << "/192 (+" << (int)p.param1 << "/192)";
				break;
			case smdEvent::WAIT_1BYTE:
				sprintf(buffer,"Wait: 0x%02X ( ",p.param1);
				os << buffer;
				os << (int)p.param1 << "/192 )";
				break;
			case smdEvent::WAIT_2BYTE:
				sprintf(buffer,"Wait: 0x%02X%02X ( ",p.param2,p.param1);
				os << buffer;
				os << (int)p.param1 + (int)p.param2*256 << "/192 )";
				break;
			case smdEvent::TRACK_END:
				os << "-- END --";
				break;
			case smdEvent::LOOP_POINT:
				os << "-- LOOP --";
				break;
			case smdEvent::SET_OCTAVE:
				os << "Octave: " << (int)p.param1 << " (real " << (int)p.param1-1 << ")";
				break;
			case smdEvent::SET_TEMPO:
				os << "Tempo: " << (int)p.param1;
				break;
			case smdEvent::SET_SAMPLE:
				os << "Sample: " << (int)p.param1;
				break;
			case smdEvent::SET_MODU:
				os << "Modulate: " << (int)p.param1;
				break;
			case smdEvent::SET_BEND:
				os << "Bend: " << (int16_t)(p.param1*256 + p.param2) << " cents";
				break;
			case smdEvent::SET_XPRESS:
				os << "Expression: " << (int)p.param1;
				break;
			case smdEvent::SET_VOLUME:
				os << "Volume: " << (int)p.param1;
				break;
			case smdEvent::SET_PAN:
				os << "Pan: " << (int)p.param1-0x40;
				break;
			default:
				sprintf(buffer,"UNK_%2X",p.eventCode);
				os << buffer;
				if(p.usedParam1)
					os << ": param1=" << (int)p.param1;
				if(p.usedParam2)
					os << ", param2=" << (int)p.param2;
				break;
		}
	os << endl;
	return os;
}

smdEvent::EVENT_TYPE smdEvent::GetType() const {
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
		case SET_XPRESS:
		case SET_VOLUME:
		case SET_PAN:
			return (EVENT_TYPE)eventCode;
		default:
			return GENERAL_UNKNOWN;
	}
}

int smdEvent::GetEventCode() const {
	return eventCode;
}

int smdEvent::GetParam1() const {
	if(usedParam1)
		return param1;
	else
		return -1;
}

int smdEvent::GetParam2() const {
	if(usedParam2)
		return param2;
	else
		return -1;
}

int smdEvent::GetParam3() const {
	if(usedParam3)
		return param3;
	else
		return -1;
}

int smdEvent::TickLength() const {
	switch(eventCode) {
		case 0x80:
			return 96;	// 1/2 note
		case 0x81:
			return 72;	// 1/4 dot
		case 0x82:
			return 64;	// 1/2 trip
		case 0x83:
			return 48;	// 1/4 note
		case 0x84:
			return 36;	// 1/8 dot
		case 0x85:
			return 32;	// 1/4 trip
		case 0x86:
			return 24;	// 1/8 note
		case 0x87:
			return 18;	// TODO
		case 0x88:
			return 16;	// 1/8 trip
		case 0x89:
			return 12;	// 1/16 note
		case 0x8A:
			return 9;	// TODO
		case 0x8B:
			return 8;	// 1/16 trip
		case 0x8C:
			return 6;	// 1/32 note
		case 0x8D:
			return 4;	// Changed from 5 for loop sync checks
		case 0x8E:
			return 3;	// TODO Was 4, now 3 for consistancy
		case 0x8F:
			return 2;	// Changed from 3 for loop sync checks
		case WAIT_AGAIN:
		case WAIT_ADD:
			return waitAgainLength;
		case WAIT_1BYTE:
			return param1;
		case WAIT_2BYTE:
			return param1 + param2*256;
		default:
			return 0;
	}
}

bool smdTrack::IsDrum() const {
	for(vector< smdEvent >::const_iterator it=events.begin();it!=events.end();++it)
		if((*it).GetType() == smdEvent::SET_SAMPLE)
			if((*it).param1 >= 0x7C)
				return true;
	return false;
}

int smdTrack::MIDI_channel() const {
	if(GetOutput()==0)
		return 0;
	if(IsDrum())
		return 9;
	int ch=GetOutput();
	if(ch>=9)
		ch++;
	return ch;
}

int smdTrack::GetAAField() const {
	return aaField;
}

int smdSong::FirstFreeOutput() const {
	bool inUse[16];
	for(int i=0;i<15;i++)
		inUse[i]=false;
	inUse[9]=true;
	for(int i=1;i<17;i++)
		if(tracks[i])
			inUse[tracks[i]->MIDI_channel()]=true;
	for(int i=0;i<15;i++)
		if(!inUse[i])
			return i;
	return 9;
}
