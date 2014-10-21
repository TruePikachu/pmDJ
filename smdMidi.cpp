#include "smdMidi.hpp"
#include "instMap.hpp"
#include "midiFile.hpp"
#include "smdFile.hpp"
#include <cstdio>
#include <vector>
using namespace std;

smdMidi::smdMidi() : MidiFile() {
	absoluteWriteTime = 0;
}

smdMidi& smdMidi::AddToFile(const smdSong& song, int loops) {
	InstrumentMap im;
	loops++;
	int latestPosition = absoluteWriteTime;
	for(vector< smdTrack >::const_iterator track=song.Tracks().begin();track!=song.Tracks().end();++track) {
		// Figure out what MIDI channel to use
		int midiChannel = track->GetOutputID();
		if(midiChannel>=9)
			midiChannel++;
		if(midiChannel==16)
			for(midiChannel=0;midiChannel<16;midiChannel++)
				if((midiChannel!=9) && !song.OutputInUse(midiChannel))
					break;
		if(track->IsDrum())
			midiChannel=9;
		// Ensure there is a track in the MIDI file corresponding to that
		// in the SMD
		if(Tracks().size()==track->GetTrackID())
			Tracks().push_back(MidiTrack());
		MidiTrack& mTrk = Tracks().operator[](track->GetTrackID());
		// Copy each event over from the SMD
		int current_time = absoluteWriteTime;
		int current_octave;
		int current_length;
		char sprintfBuffer[256];
		bool doesLoop = false;
		int sampleOctOff = 0;
		int instGroup = song.GetInstrumentGroup();
		vector< smdEvent >::const_iterator loopToPos=track->Events.begin();

		for(vector< smdEvent >::const_iterator event=track->Events.begin();loops;++event)
			switch(event->GetType()) {
				case smdEvent::NOTE_PLAY:
					{
						char eventFlags = event->Param(0) >> 4;
						switch(eventFlags & 0x3) {
							case 0x0:
								current_octave--;
							case 0x1:
								current_octave--;
							case 0x2:
								break;
							case 0x3:
								current_octave++;
						}
						switch(eventFlags & 0xC) {
							case 0x4:
								current_length = event->Param(1);
							case 0x0:
								break;
							case 0x8:
								current_length = event->Param(1)*256 + event->Param(2);
						}
						int key = (sampleOctOff + current_octave)*12 + event->Param(0)&0xF;
						if(midiChannel==9)
							key=im.MapDrumKey(instGroup,key);
						mTrk.Events().push_back(MidiEvent(current_time,MidiEvent::NOTE_ON,midiChannel,key,event->GetEventCode()));
						mTrk.Events().push_back(MidiEvent(current_time+current_length,MidiEvent::NOTE_OFF,midiChannel,key,event->GetEventCode()));
					}
					break;
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
							event=loopToPos;
						else
							loops=0;
					break;
				case smdEvent::LOOP_POINT:
					mTrk.Events().push_back(MidiEvent(current_time,0x6,4,"LOOP"));
					loopToPos = event;
					doesLoop = true;
					break;
				case smdEvent::SET_OCTAVE:
					current_octave = event->Param(0);
					break;
				case smdEvent::SET_TEMPO:
					{
						long unsigned mpqn = 60000000/event->Param(0);
						char buf[3];
						buf[0] = (mpqn >> 16) & 0xFF;
						buf[1] = (mpqn >>  8) & 0xFF;
						buf[2] = (mpqn >>  0) & 0xFF;
						mTrk.Events().push_back(MidiEvent(current_time,0x51,3,buf));
					} break;
				case smdEvent::SET_SAMPLE:
					mTrk.Events().push_back(MidiEvent(current_time,MidiEvent::PROGRAM,midiChannel,im.MapInstrument(instGroup,event->Param(0)),0));
					sampleOctOff = im.MapInstrumentOff(instGroup,event->Param(0));
					break;
				case smdEvent::SET_MODU:
					mTrk.Events().push_back(MidiEvent(current_time,MidiEvent::CONTROLLER,midiChannel,0x01,event->Param(0)));
					break;
				case smdEvent::SET_BEND:
					if(instGroup==108)	// TODO what is this for? What is group 108?
						break;
					{
						int bendParam = 0x2000 + (int16_t)(event->Param(0)*256 + event->Param(1));
						mTrk.Events().push_back(MidiEvent(current_time,MidiEvent::BEND,midiChannel,bendParam));
					}
					break;
				case smdEvent::SET_VOLUME:
					mTrk.Events().push_back(MidiEvent(current_time,MidiEvent::CONTROLLER,midiChannel,0x07,event->Param(0)));
					break;
				case smdEvent::SET_XPRESS:
					mTrk.Events().push_back(MidiEvent(current_time,MidiEvent::CONTROLLER,midiChannel,0x0B,event->Param(0)));
					break;
				case smdEvent::SET_PAN:
					mTrk.Events().push_back(MidiEvent(current_time,MidiEvent::CONTROLLER,midiChannel,0x0A,event->Param(0)));
					break;
				default:
					switch(event->GetParamCount()) {
						case 2:
							sprintf(sprintfBuffer,"UNK_%02X 0x%02X 0x%02X",event->GetEventCode(),event->Param(0),event->Param(1));
							break;
						case 1:
							sprintf(sprintfBuffer,"UNK_%02X 0x%02X",event->GetEventCode(),event->Param(0));
							break;
						case 0:
							sprintf(sprintfBuffer,"UNK_%02X",event->GetEventCode());
							break;
					}
					mTrk.Events().push_back(MidiEvent(current_time,0x6,strlen(sprintfBuffer),sprintfBuffer));
			}
		if(latestPosition<current_time)
			latestPosition=current_time;
	}
	absoluteWriteTime = latestPosition;
	return *this;
}
