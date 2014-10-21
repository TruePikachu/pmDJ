#include "midiFile.hpp"
#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>
using namespace std;

void readFlipped(std::istream &is, char* data, size_t nData) {
	char buffer[nData];
	is.read(buffer,nData);
	for(int i=0;i<nData;i++)
		data[i]=buffer[nData-i-1];
}

void writeFlipped(std::ostream& os, const char* data, size_t nData) {
	for(int i=nData-1;i>=0;i--)
		os.write(data+i,1);
}

long long int readFlipped(std::istream &is, size_t nData) {
	long long int result = 0;
	readFlipped(is,(char*)&result,nData);
	return result;
}

void writeFlipped(std::ostream &os, long long int data, size_t nData) {
	writeFlipped(os,(const char *)&data,nData);
}

long int readVarLength(std::istream &is) {
	long int result = 0;
	uint8_t buffer;
	do {
		is.read((char*)&buffer,1);
		result <<= 7;
		result += buffer&0x7F;
	} while (buffer&0x80);
	return result;
}

void writeVarLength(std::ostream &os, long int data) {
	char buffer[4];
	for(int i=3;i>=0;i--) {
		buffer[i]=data&0x7F;
		data>>=7;
	}
	int i;
	for(i=0;i<4;i++)
		if(buffer[i])
			break;
	for(;i<3;i++)
		writeFlipped(os,buffer[i]|0x80,1);
	writeFlipped(os,buffer[3],1);
}

//////////

MidiFile::MidiFile() {
}

MidiFile::MidiFile(std::ifstream& file) {
	// Check header
	char buffer[32];
	file.read(buffer,4);
	if(strncmp(buffer,"MThd",4))
		throw runtime_error("Not a MIDI file");
	readFlipped(file,buffer,4);
	if(1!=readFlipped(file,2))
		throw runtime_error("Only type 1 files supported");
	int trackCount = readFlipped(file,2);
	if(48!=readFlipped(file,2))
		throw runtime_error("Only 48 tick/beat files supported");
	for(int i=0;i<trackCount;i++) {
		MidiTrack myTrack;
		myTrack.Read(file);
		tracks.push_back(myTrack);
	}
}

std::vector< MidiTrack >& MidiFile::Tracks() {
	return tracks;
}

const std::vector< MidiTrack >& MidiFile::Tracks() const {
	return tracks;
}

const MidiFile& MidiFile::Save(std::ofstream& file) const {
	// Write header
	file.write("MThd",4);
	writeFlipped(file,6,4);
	writeFlipped(file,1,2);
	writeFlipped(file,tracks.size(),2);
	writeFlipped(file,48,2);
	// Write tracks
	for(vector< MidiTrack >::const_iterator it=tracks.begin();it!=tracks.end();++it)
		it->Write(file);
	return *this;
}

//////////

const MidiTrack& MidiTrack::Write(std::ofstream& file) const {
	// Write header
	file.write("MTrk",4);
	off_t sizeOff = file.tellp();
	writeFlipped(file,0xDEADBEEF,4);
	// Sort events
	vector< MidiEvent > sorted = events;
	sort(sorted.begin(),sorted.end());
	// Write events
	MidiEvent::wroteTime = 0;
	for(vector< MidiEvent >::const_iterator it=sorted.begin();it!=sorted.end();++it)
		it->Write(file);
	// Write TRACKEND
	MidiEvent TrackEnd(MidiEvent::wroteTime,0x2F,0,"");
	if(TrackEnd.absoluteTime < padToTime)
		TrackEnd.absoluteTime = padToTime;
	TrackEnd.Write(file);
	// Write size
	off_t endOff = file.tellp();
	file.seekp(sizeOff);
	writeFlipped(file,endOff-sizeOff-4,4);
	file.seekp(endOff);
	return *this;
}

MidiTrack& MidiTrack::Read(std::ifstream& file) {
	// Read header
	char buffer[64];
	file.read(buffer,4);
	if(strncmp(buffer,"MTrk",4))
		throw runtime_error("Bad MIDI track header");
	size_t trackLength = readFlipped(file,4);
	off_t trackDataStart = file.tellg();
	MidiEvent::wroteTime=0;
	do {
		MidiEvent myEvent(file);
		events.push_back(myEvent);
	} while (trackDataStart + trackLength > file.tellg());
	if(trackDataStart+trackLength<file.tellg())
		throw runtime_error("Read past end of track wrt size");
	if(events[events.size()-1].eventType!=MidiEvent::META)
		throw runtime_error("Track doesn't end with TRACK_END");
	if(events[events.size()-1].param1!=0x2F)
		throw runtime_error("Track doesn't end with TRACK_END");
	events.pop_back();
	return *this;
}

MidiTrack::MidiTrack() : padToTime(0) {
}

std::vector< MidiEvent >& MidiTrack::Events() {
	return events;
}

const std::vector< MidiEvent >& MidiTrack::Events() const {
	return events;
}

MidiTrack& MidiTrack::AddEvent(MidiEvent that) {
	events.push_back(that);
	return *this;
}

int MidiTrack::GetPadTime() const {
	return padToTime;
}

MidiTrack& MidiTrack::SetPadTime(int i) {
	padToTime = i;
	return *this;
}

//////////
int MidiEvent::wroteTime;
long int MidiEvent::makeIndex = 0;

const MidiEvent& MidiEvent::Write(std::ofstream& file) const {
	writeVarLength(file,absoluteTime-wroteTime);
	wroteTime=absoluteTime;
	switch(eventType) {
		case NOTE_ON:
			writeFlipped(file,0x90|channel,1);
			file.write((const char*)&param1,1);
			file.write((const char*)&param2,1);
			return *this;
		case NOTE_OFF:
			writeFlipped(file,0x80|channel,1);
			file.write((const char*)&param1,1);
			file.write((const char*)&param2,1);
			return *this;
		case CONTROLLER:
			writeFlipped(file,0xB0|channel,1);
			file.write((const char*)&param1,1);
			file.write((const char*)&param2,1);
			return *this;
		case PROGRAM:
			writeFlipped(file,0xC0|channel,1);
			file.write((const char*)&param1,1);
			return *this;
		case BEND:
			writeFlipped(file,0xE0|channel,1);
			file.write((const char*)&param1,1);
			file.write((const char*)&param2,1);
			return *this;
		case META:
			writeFlipped(file,255,1);
			file.write((const char*)&param1,1);
			writeVarLength(file,metaLength);
			file.write(metaData,metaLength);
			return *this;
	}
}

MidiEvent::MidiEvent(std::ifstream &file) {
	char buffer[64];
	wroteTime += readVarLength(file);
	absoluteTime=wroteTime;
	char byte1[1];
	file.read(byte1,1);
	channel = byte1[0]&0xF;
	if((uint8_t)byte1[0]==0xFF) {
		eventType=META;
		param1=readFlipped(file,1);
		metaLength=readVarLength(file);
		metaData = new char[metaLength];
		file.read(metaData,metaLength);
		return;
	}
	switch(byte1[0]&0xF0) {
		case 0x80:
			eventType=NOTE_OFF;
			break;
		case 0x90:
			eventType=NOTE_ON;
			break;
		case 0xB0:
			eventType=CONTROLLER;
			break;
		case 0xC0:
			eventType=PROGRAM;
			break;
		case 0xE0:
			eventType=BEND;
			break;
		default:
			throw runtime_error("Event type not supported");
	}
	switch(eventType) {
		case NOTE_ON:
		case NOTE_OFF:
		case CONTROLLER:
		case BEND:
			param1=readFlipped(file,1);
			param2=readFlipped(file,1);
			break;
		case PROGRAM:
			param1=readFlipped(file,1);
			break;
	}
}

MidiEvent::MidiEvent(int absoluteTime,EventType eventType, uint8_t channel, uint8_t param1, uint8_t param2) : madeIndex(makeIndex++), absoluteTime(absoluteTime), eventType(eventType), channel(channel), param1(param1), param2(param2) {
	switch(eventType) {
		case NOTE_ON:
		case NOTE_OFF:
		case CONTROLLER:
			break;
		default:
			throw logic_error("Wrong MidiEvent::MidiEvent() for eventType");
	}
}

MidiEvent::MidiEvent(int absoluteTime,EventType eventType, uint8_t channel, uint16_t param) : madeIndex(makeIndex++), absoluteTime(absoluteTime), eventType(eventType), channel(channel), param1((param)&0x7F), param2((param>>7)&0x7F) {
	switch(eventType) {
		case PROGRAM:
		case BEND:
			break;
		default:
			throw logic_error("Wrong MidiEvent::MidiEvent() for eventType");
	}
}

MidiEvent::MidiEvent(int absoluteTime,uint8_t metaType,size_t metaLength,const char * mData) : madeIndex(makeIndex++), eventType(META), absoluteTime(absoluteTime), param1(metaType), metaLength(metaLength) {
	metaData = new char[metaLength];
	memmove(metaData,mData,metaLength);
}

MidiEvent::MidiEvent(const MidiEvent& other) : madeIndex(other.madeIndex), absoluteTime(other.absoluteTime), eventType(other.eventType), channel(other.channel), param1(other.param1), param2(other.param2), metaLength(other.metaLength) {
	if(eventType==META) {
		metaData = new char[metaLength];
		memmove(metaData,other.metaData,metaLength);
	}
}

MidiEvent::~MidiEvent() {
	if(eventType==META)
		delete metaData;
}

MidiEvent& MidiEvent::operator=(MidiEvent other) {
	madeIndex=other.madeIndex;
	absoluteTime=other.absoluteTime;
	eventType=other.eventType;
	channel=other.channel;
	param1=other.param1;
	param2=other.param2;
	metaLength=other.metaLength;
	if(eventType==META) {
		metaData = new char[metaLength];
		memmove(metaData,other.metaData,metaLength);
	}
	return *this;
}

MidiEvent::EventType MidiEvent::GetType() const {
	return eventType;
}

bool MidiEvent::operator<(const MidiEvent&other) const {
	if(absoluteTime<other.absoluteTime)
		return true;
	if(absoluteTime>other.absoluteTime)
		return false;
	if((eventType==NOTE_OFF)&&(other.eventType==NOTE_ON))
		return true;
	if((eventType==NOTE_ON)&&(other.eventType==NOTE_OFF))
		return false;
	return madeIndex<other.madeIndex;
}
