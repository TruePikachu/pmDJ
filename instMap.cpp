#include "instMap.hpp"
#include <cstdio>
#include <iostream>
#include <map>
using namespace std;

InstrumentMap::InstrumentMap() {
	// default programs
	FILE* pFile = fopen(PKGDIR "/prog_table.txt","r");
	if(pFile) {
		while(!feof(pFile)) {
			int aa,instID,val3,val4;
			fscanf(pFile,"%X %X %X %d",&aa,&instID,&val3,&val4);
			if(!aa)
				for(int i=0;i<128;i++) {
					InstMap[i][instID]=val3;
					OctMap[i][instID]=val4;
				}
		}
		fclose(pFile);
	}
	// prog_table
	pFile = fopen(PKGDIR "/prog_table.txt","r");
	if(pFile) {
		while(!feof(pFile)) {
			int aa,instID,val3,val4;
			fscanf(pFile,"%X %X %X %d",&aa,&instID,&val3,&val4);
			InstMap[aa][instID]=val3;
			OctMap[aa][instID]=val4;
		}
		fclose(pFile);
		instMapOK=true;
	} else
		instMapOK=false;
	// drum_table
	pFile = fopen(PKGDIR "/drum_table.txt","r");
	if(pFile) {
		while(!feof(pFile)) {
			int aa,inKey,outKey;
			fscanf(pFile,"%X %X %X",&aa,&inKey,&outKey);
			DrumKeyMap[aa][inKey] = outKey;
		}
		fclose(pFile);
		drumMapOK=true;
	} else
		drumMapOK=false;
}

int InstrumentMap::MapDrumKey(int aa, int key) {
	if(!drumMapOK)
		return key;
	if(!DrumKeyMap[aa][key])
		return key;
	return DrumKeyMap[aa][key];
}

int InstrumentMap::MapInstrument(int aa, int id) {
	if(!instMapOK)
		return id;
	return InstMap[aa][id];
}

int InstrumentMap::MapInstrumentOff(int aa, int id) {
	if(!instMapOK)
		return 0;
	return OctMap[aa][id];
}
