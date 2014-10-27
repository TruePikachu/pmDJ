#ifndef __SMDFILE_HPP
#define __SMDFILE_HPP
class smdSong;
class smdTrack;
class smdEvent;
#include <fstream>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

// TODO Empty constructors and data-writing functions are not prototyped until
// we have a lot more data on unknown stuff

class smdSong {
	private:
		std::string		name;
		std::vector< smdTrack >	tracks;
		int			instrumentGroup;
		friend std::ostream& operator<<(std::ostream&, const smdSong&);
	public:
						 smdSong	(std::ifstream&);
		std::string			GetName		() const;
		int				GetInstrumentGroup() const;
		int				GetTrackCount	() const;
		const std::vector< smdTrack >&	Tracks		() const;
		const smdTrack&			operator[]	(int) const;
		bool				OutputInUse	(int) const;
		bool			OutputInUseNotDrum	(int) const;
};

class smdTrack {
	private:
		friend class smdSong;
		int				trackID;
		int				outputID;
		int				instrumentGroup;
		std::vector< smdEvent >		events;
						 smdTrack(std::ifstream&,int);
		friend std::ostream& operator<<(std::ostream&, const smdTrack&);

	public:
		int				GetTrackID	() const;
		int				GetOutputID	() const;
		int				GetInstrumentGroup() const;
		bool				IsDrum		() const;
		int				GetEventCount	() const;
		const std::vector< smdEvent >&	Events		() const;
		const smdEvent&			operator[]	(int) const;
		size_t				LongestCmdSize	() const;
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
		} EventType;
	private:
		friend class smdTrack;
		static int prevWaitLength;
		uint8_t eventCode;
		std::vector< uint8_t > params;
		int waitAgainLength;
				 smdEvent	(std::ifstream&);
		friend std::ostream& operator<<(std::ostream&, const smdEvent&);
	public:
		static size_t	DisplayBytes;
		EventType	GetType		() const;
		uint8_t		GetEventCode	() const;
		int		GetParamCount	() const;
		uint8_t		Param		(int) const;
		int		TickLength	() const;
		size_t		CmdSize		() const;
};

#endif
