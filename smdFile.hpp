#ifndef __SMDFILE_HPP
#define __SMDFILE_HPP
class SmdSong;
class SmdTrack;
class SmdEvent;
#include <fstream>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

// TODO Empty constructors and data-writing functions are not prototyped until
// we have a lot more data on unknown stuff

class SmdSong {
	private:
		std::string		name;
		std::vector< SmdTrack >	tracks;
		int			instrumentGroup;
		friend std::ostream& operator<<(std::ostream&, const SmdSong&);
	public:
						 smdSong	(std::ifstream&);
		std::string			GetName		() const;
		int				GetInstrumentGroup() const;
		int				GetTrackCount	() const;
		const std::vector< SmdTrack >&	Tracks		() const;
		const smdTrack&			operator[]	(int) const;
		bool				OutputInUse	(int) const;
};

class SmdTrack {
	private:
		friend class smdSong;
		int				trackID;
		int				outputID;
		int				instrumentGroup;
		std::vector< SmdEvent >		events;
						 smdTrack(std::ifstream&);
		friend std::ostream& operator<<(std::ostream&, const SmdTrack&);

	public:
		int				GetTrackID	() const;
		int				GetOutputID	() const;
		int				GetInstrumentGroup() const;
		bool				IsDrum		() const;
		int				GetEventCount	() const;
		const std::vector< SmdEvent >&	Events		() const;
		const SmdEvent&			operator[]	(int) const;
};

class SmdEvent {
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
		friend std::ostream& operator<<(std::ostream&, const SmdEvent&);
	public:
		EVENT_TYPE	GetType		() const;
		uint8_t		GetEventCode	() const;
		int		GetParamCount	() const;
		uint8_t		Param		(int) const;
		int		TickLength	() const;
};

#endif
