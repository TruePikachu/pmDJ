#include "instMap.hpp"
#include "midiFile.hpp"
#include "smdRead.hpp"
#include <algorithm>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>
using namespace std;

long int MidiEvent::makeIndex =0;
int MidiEvent::lastSentRPN =0;
static InstrumentMap IMap;

std::ofstream& WriteByte(std::ofstream& file, uint8_t n) {
	char buf = n;
	file.write(&buf,1);
	return file;
}

std::ofstream& WriteWord(std::ofstream& file, uint16_t n) {
	WriteByte(file,(n&0xFF00)>>8);
	WriteByte(file,(n&0x00FF)>>0);
	return file;
}

std::ofstream& WriteDWord(std::ofstream& file, uint32_t n) {
	WriteWord(file,(n&0xFFFF0000)>>16);
	WriteWord(file,(n&0x0000FFFF)>> 0);
	return file;
}

std::ofstream& WriteLong(std::ofstream& file, unsigned long n) {
	char buffer[4];
	size_t dataLen = EncodeVarLen(n,sizeof(buffer),buffer);
	for(int i=sizeof(buffer)-dataLen;i<sizeof(buffer);i++)
		WriteByte(file,buffer[i]);
	if(dataLen==0)
		WriteByte(file,0);
	return file;
}

size_t EncodeVarLen(unsigned long source, size_t destByteCount, char *pData) {
	size_t bytesUsed = 0;
	for(int i=destByteCount-1;i>=0;i--) {
		if(source)
			bytesUsed++;
		pData[i] = source & 0x7F;
		source >>= 7;
	}
	for(int i=0;i<destByteCount-1;i++)
		pData[i] |= 0x80;
	return bytesUsed;
}

MidiFile& MidiFile::LoadSMD(const smdSong& song, unsigned loops) {
	if(!loops)
		loops=1;
	for(int tID=0;tID<17;tID++)
		if(song[tID])
			tracks.push_back(MidiTrack(*song[tID],loops,song.FirstFreeOutput()));
	return *this;
}

bool MidiFile::WriteFile(std::ofstream& file) const {
	// Header chunk
	file.write("MThd",4);
	WriteDWord(file,6);
	WriteWord(file,1);
	WriteWord(file,tracks.size());
	WriteWord(file,48);

	// Track chunks
	for(vector< MidiTrack >::const_iterator it=tracks.begin();it!=tracks.end();++it)
		if(!(*it).WriteFile(file))
			return false;
	return true;
}

bool MidiTrack::WriteFile(std::ofstream& file) const {
	// Sort the events in the list of events
	vector< MidiEvent > sortedEvents = events;
	sort(sortedEvents.begin(),sortedEvents.end());
	
	// Header
	file.write("MTrk",4);
	off_t dataSizeLoc = file.tellp();
	WriteDWord(file,0xEFBEADDE);

	// Data
	sortedEvents[0].WriteFile(file,0);
	for(vector< MidiEvent >::const_iterator it=sortedEvents.begin()+1;it!=sortedEvents.end();++it)
		if(!(*it).WriteFile(file,(*(it-1)).absoluteTime))
			return false;
	off_t dataEndLoc = file.tellp();
	file.seekp(dataSizeLoc);
	WriteDWord(file,dataEndLoc-dataSizeLoc-4);
	file.seekp(dataEndLoc);
	return true;
}

MidiTrack::MidiTrack(const smdTrack&track,unsigned loops,int freeOut) {
	if(!loops)
		loops=1;
	int current_time	=  0;
	int current_octave	= -1;
	int current_length	= -1;
	int output_id		= track.MIDI_channel();
	if(output_id==16)
		output_id = freeOut;
	int lastNoteOffTime	= 0;
	char sprintfBuffer[256];
	bool doesLoop=false;

	int sampleOctOff = 0;
	int aa_field = track.GetAAField();
	vector< smdEvent >::const_iterator loopToPos=track.GetEvents().begin();

	for(vector< smdEvent >::const_iterator it=track.GetEvents().begin();loops;++it) {
		switch((*it).GetType()) {
			case smdEvent::NOTE_PLAY:
				{	// Handle the high nibble of the event
				char eventFlags = (*it).GetParam1() >> 4;
				// Low 2 bits indicate octave change
				switch(eventFlags & 0x3) {
					case 0x0:
						current_octave--;
					case 0x1:
						current_octave--;
						if(current_octave < -1) {
							cerr << "WARNING: current_octave brought below -1; clamping (output ID " << output_id+1 << ")\n";
							current_octave = -1;
						}
					case 0x2:
						break;
					case 0x3:
						current_octave++;
						if(current_octave > 9) {
							cerr << "WARNING: current_octave brought above 9; clamping (output ID " << output_id+1 << ")\n";
							current_octave = 9;
						}
						break;
					default:
						throw ENOTSUP;
				}
				// I'll just say, for now, upper 2 bits flag length change
				switch(eventFlags & 0xC) {
					case 0x4:
						current_length = (*it).GetParam2();
					case 0x0:
						break;
					case 0x8:
						current_length = (*it).GetParam2()*256 + (*it).GetParam3();
						break;
					default:
						throw ENOTSUP;
				}
				// Note on and off events
				int key = (sampleOctOff+current_octave)*12 + 12 + ((*it).GetParam1() & 0xF);
				if(output_id==9)
					key=IMap.MapDrumKey(aa_field,key);
				events.push_back(MidiEvent(current_time,MidiEvent::NOTE_ON,output_id,key,(*it).GetEventCode()));
				events.push_back(MidiEvent(current_time+current_length,MidiEvent::NOTE_OFF,output_id,key,(*it).GetEventCode()));
				if(lastNoteOffTime < (current_time + current_length))
					lastNoteOffTime = current_time+current_length;
				} break;
			case smdEvent::DELTA_TIME:
			case smdEvent::WAIT_AGAIN:
			case smdEvent::WAIT_ADD:
			case smdEvent::WAIT_2BYTE:
			case smdEvent::WAIT_1BYTE:
				current_time += (*it).TickLength();
				break;
			case smdEvent::TRACK_END:
				if(--loops)
					if(doesLoop)
						it=loopToPos;
					else
						loops=0;
				break;
			case smdEvent::LOOP_POINT:
				events.push_back(MidiEvent(current_time,0x6,4,"LOOP"));
				loopToPos=it;
				doesLoop=true;
				break;
			case smdEvent::SET_OCTAVE:
				current_octave = (*it).GetParam1() - 1;
				break;
			case smdEvent::SET_TEMPO:
				{
					long unsigned mpqn = 60000000 / (*it).GetParam1();
					char buf[3];
					buf[0] = (mpqn >> 16) & 0xFF;
					buf[1] = (mpqn >>  8) & 0xFF;
					buf[2] = (mpqn >>  0) & 0xFF;
					events.push_back(MidiEvent(current_time,0x51,3,buf));
				} break;
			case smdEvent::SET_SAMPLE:
				events.push_back(MidiEvent(current_time,MidiEvent::PROGRAM,output_id,::IMap.MapInstrument(aa_field,(*it).GetParam1()),0));
				sampleOctOff = ::IMap.MapInstrumentOff(aa_field,(*it).GetParam1());
				break;
			case smdEvent::SET_MODU:
				events.push_back(MidiEvent(current_time,MidiEvent::CONTROLLER,output_id,0x01,(*it).GetParam1()));
				break;
			case smdEvent::SET_BEND:
				if(aa_field == 108)
					break;
				{
					int bendParam = 0x2000 + (int16_t)((*it).GetParam1()*256 + (*it).GetParam2());
					events.push_back(MidiEvent(current_time,MidiEvent::BEND,output_id,(bendParam)&0x7F,(bendParam>>7)&0x7F));
				}
				break;
			case smdEvent::SET_XPRESS:
				events.push_back(MidiEvent(current_time,MidiEvent::CONTROLLER,output_id,0x0B,(*it).GetParam1()));
				break;
			case smdEvent::SET_VOLUME:
				events.push_back(MidiEvent(current_time,MidiEvent::CONTROLLER,output_id,0x07,(*it).GetParam1()));
				break;
			case smdEvent::SET_PAN:
				events.push_back(MidiEvent(current_time,MidiEvent::CONTROLLER,output_id,0x0A,(*it).GetParam1()));
				break;
			default:
				if((*it).GetParam2()!=-1)
					sprintf(sprintfBuffer,"UNK_%02X 0x%02X 0x%02X",(*it).GetEventCode(),(*it).GetParam1(),(*it).GetParam2());
				else if((*it).GetParam1()!=-1)
					sprintf(sprintfBuffer,"UNK_%02X 0x%02X",(*it).GetEventCode(),(*it).GetParam1());
				else
					sprintf(sprintfBuffer,"UNK_%02X",(*it).GetEventCode());
				events.push_back(MidiEvent(current_time,0x6,strlen(sprintfBuffer),sprintfBuffer));
		}
	}
	if(lastNoteOffTime>current_time)
		current_time=lastNoteOffTime;
	events.push_back(MidiEvent(current_time,0x2F,0,""));
}

bool MidiEvent::WriteFile(std::ofstream&file,int prev) const {
	// Delta time
	WriteLong(file,absoluteTime-prev);
	// Event type
	switch(eventType) {
		case META_EVENT:
			WriteByte(file,0xFF);
			WriteByte(file,param1);
			WriteLong(file,metaLength);
			file.write(metaData,metaLength);
			return true;
		case NOTE_ON:
			WriteByte(file,0x90 | channel);
			WriteByte(file,param1);
			WriteByte(file,param2);
			return true;
		case NOTE_OFF:
			WriteByte(file,0x80 | channel);
			WriteByte(file,param1);
			WriteByte(file,param2);
			return true;
		case CONTROLLER:
			WriteByte(file,0xB0 | channel);
			WriteByte(file,param1);
			WriteByte(file,param2);
			return true;
		case BEND:
			WriteByte(file,0xE0 | channel);
			WriteByte(file,param1);
			WriteByte(file,param2);
			return true;
		case PROGRAM:
			WriteByte(file,0xC0 | channel);
			WriteByte(file,param1);
			return true;
		case SET_RPN:
			if(param1!=lastSentRPN) {
			 // RPN coarse
			 WriteByte(file,0xB0 | channel);
			 WriteByte(file,0x65);
			 WriteByte(file,0x00);
			 // RPN fine
			 WriteLong(file,0);
			 WriteByte(file,0xB0 | channel);
			 WriteByte(file,0x64);
			 WriteByte(file,param1);
			 // Data coarse
			 WriteLong(file,0);
			}
			WriteByte(file,0xB0 | channel);
			WriteByte(file,0x06);
			WriteByte(file,(rpnData>>7) & 0x7F);
			// Data fine
			WriteLong(file,0);
			WriteByte(file,0xB0 | channel);
			WriteByte(file,0x26);
			WriteByte(file,(rpnData>>0) & 0x7F);
			lastSentRPN=param1;
			return true;
		default:
			return false;
	}
}

MidiEvent::MidiEvent(int time, EventType type, uint8_t ch, uint8_t p1, uint8_t p2) {
	madeIndex = makeIndex++;
	absoluteTime = time;
	eventType = type;
	channel = ch;
	param1 = p1;
	param2 = p2;
	metaData = 0;
}

MidiEvent::MidiEvent(int time, uint8_t mType, size_t mLen, const char * mDat) {
	madeIndex = makeIndex++;
	absoluteTime = time;
	eventType = META_EVENT;
	param1 = mType;
	metaLength = mLen;
	metaData = new char[mLen];
	memcpy(metaData,mDat,mLen);
}

MidiEvent::MidiEvent(const MidiEvent&them) {
	madeIndex = them.madeIndex;
	absoluteTime = them.absoluteTime;
	eventType = them.eventType;
	channel = them.channel;
	param1 = them.param1;
	param2 = them.param2;
	metaLength = them.metaLength;
	if(them.metaData) {
		metaData = new char[metaLength];
		memcpy(metaData,them.metaData,metaLength);
	} else
		metaData = 0;
}

MidiEvent::~MidiEvent() {
	if(metaData)
		delete metaData;
}

MidiEvent& MidiEvent::operator=(MidiEvent them) {
	madeIndex = them.madeIndex;
	absoluteTime = them.absoluteTime;
	eventType = them.eventType;
	channel = them.channel;
	param1 = them.param1;
	param2 = them.param2;
	metaLength = them.metaLength;
	if(them.metaData) {
		metaData = new char[metaLength];
		memcpy(metaData,them.metaData,metaLength);
	} else
		metaData = 0;
	return *this;
}

bool MidiEvent::operator<(const MidiEvent& rhs) const {
	if(absoluteTime < rhs.absoluteTime)
		return true;
	if(absoluteTime > rhs.absoluteTime)
		return false;
	if((eventType==META_EVENT) && (param1==0x2F))
		return false;
	if((rhs.eventType==META_EVENT) && (rhs.param1==0x2F))
		return true;
	if(eventType == NOTE_OFF && rhs.eventType == NOTE_ON)
		return true;
	if(eventType == NOTE_ON && rhs.eventType == NOTE_OFF)
		return false;
	return (madeIndex < rhs.madeIndex);
}


std::ostream& operator<<(std::ostream&os, const MidiFile&p) {
	for(vector< MidiTrack >::const_iterator it=p.tracks.begin();it!=p.tracks.end();++it)
		os << *it << endl;
	return os;
}

std::ostream& operator<<(std::ostream&os, const MidiTrack&p) {
	os << "Track:\n";
	for(vector< MidiEvent >::const_iterator it=p.events.begin();it!=p.events.end();++it)
		os << *it;
	return os;
}

std::ostream& operator<<(std::ostream&os, const MidiEvent&p) {
	os << p.absoluteTime << '\t';
	switch(p.eventType) {
		case MidiEvent::NOTE_ON:
			os << "NOTE_ON";
			break;
		case MidiEvent::NOTE_OFF:
			os << "NOTE_OFF";
			break;
		case MidiEvent::PROGRAM:
			os << "PROGRAM";
			break;
		case MidiEvent::META_EVENT:
			os << "META_EVENT";
			break;
	}
	os << endl;
	return os;
}

MidiFile& MidiFile::ApplyRemap(const uint8_t mapper[256]) {
	for(vector< MidiTrack >::iterator it=tracks.begin();it!=tracks.end();++it)
		(*it).ApplyRemap(mapper);
	return *this;
}

MidiTrack& MidiTrack::ApplyRemap(const uint8_t mapper[256]) {
	for(vector< MidiEvent >::iterator it=events.begin();it!=events.end();++it)
		(*it).RemapInstrument(mapper);
	return *this;
}

MidiEvent& MidiEvent::RemapInstrument(const uint8_t mapper[256]) {
	if(eventType == PROGRAM)
		param1 = mapper[param1];
	return *this;
}

MidiEvent::MidiEvent(int when, uint8_t chan, int rpnID, uint16_t rpnDat) {
	madeIndex = makeIndex++;
	eventType = SET_RPN;
	absoluteTime = when;
	channel = chan;
	param1 = rpnID;
	rpnData = rpnDat;
	metaData = 0;
}
