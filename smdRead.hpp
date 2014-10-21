#ifndef __SMDREAD_HPP
#define __SMDREAD_HPP
class smdSong;
class smdTrack;
class smdEvent;
#include <fstream>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

class smdSong {
	private:
		std::string	name;
		int		trackCount;
		smdTrack *	tracks[17];
		friend std::ostream& operator<<(std::ostream&, const smdSong&);
		int		aaField;
	public:
				 smdSong	(std::ifstream&,bool throwToCaller = false);
				 smdSong	(const smdSong&);
				~smdSong	();
		smdSong&	operator=	(smdSong);

		std::string	GetName		() const;
		const smdTrack*	operator[]	(int) const;
		int		GetTrackCount	() const;

		int		FirstFreeOutput	() const;

};

class smdTrack {
	private:
		friend class smdSong;
		int				trackID;
		int				outputID;
		int				aaField;
		std::vector< smdEvent >		events;
				 smdTrack	(std::ifstream&, bool throwToCaller, int aaField);
		friend std::ostream& operator<<(std::ostream&, const smdTrack&);
	public:
		int		GetID		() const;
		int		GetOutput	() const;
		bool		IsDrum		() const;
		int		MIDI_channel	() const;
		const std::vector< smdEvent >& GetEvents () const;
		int		GetEventCount	() const;
		const smdEvent&	operator[]	(int) const;
		int 		GetAAField	() const;
};

class smdEvent {
	public:
		typedef enum {
			NOTE_PLAY,
			DELTA_TIME,
			WAIT_AGAIN	= 0x90,
			WAIT_ADD	= 0x91,
			WAIT_1BYTE	= 0x92,
			WAIT_2BYTE	= 0x93,
			TRACK_END	= 0x98,
			LOOP_POINT	= 0x99,
			SET_OCTAVE	= 0xA0,
			SET_TEMPO	= 0xA4,
			SET_SAMPLE	= 0xAC,
			SET_MODU	= 0xBE,
			SET_BEND	= 0xD7,
			SET_VOLUME	= 0xE0,
			SET_XPRESS	= 0xE3,
			SET_PAN		= 0xE8,
			GENERAL_UNKNOWN
		} EVENT_TYPE;
	private:
		static int prevWaitLength;
		friend class smdTrack;
		uint8_t		eventCode;
		bool		usedParam1;
		uint8_t		param1;
		bool		usedParam2;
		uint8_t		param2;
		bool		usedParam3;
		uint8_t		param3;
		int		waitAgainLength;
				 smdEvent	(std::ifstream&);
		friend std::ostream& operator<<(std::ostream&, const smdEvent&);
	public:
		EVENT_TYPE	GetType		() const;
		int		GetEventCode	() const;
		int		GetParam1	() const;
		int		GetParam2	() const;
		int		GetParam3	() const;
		int		TickLength	() const;
};

#endif
