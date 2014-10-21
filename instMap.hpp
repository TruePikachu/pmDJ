#ifndef __INSTMAP_HPP
#define __INSTMAP_HPP
class InstrumentMap;
#include <map>

class InstrumentMap {
	private:
		std::map< int, std::map< int, int> > DrumKeyMap;
		std::map< int, std::map< int, int> > InstMap;
		std::map< int, std::map< int, int> > OctMap;
	public:
		InstrumentMap();
		int	MapDrumKey(int,int);
		int	MapInstrument(int,int);
		int	MapInstrumentOff(int,int);
};

#endif
