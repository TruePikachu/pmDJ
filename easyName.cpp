#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
	if(argc!=3) {
		cout << "Usage: " << argv[0] << " <EoS.nds> <path/to/*.smd>\n";
		cout << "(if you have ~/dir/bgm0001.smd, use '~/dir')\n";
		return 0;
	}
	// Open the ROM
	ifstream rom(argv[1]);
	for(int i=1;i<=141;i++) {
		char buf[64];
		// Print jukebox ID number
		sprintf(buf,"%3i. ",i);
		cout << buf;
		// Get the name from the ROM
		rom.seekg(0x20771F0+4*i);
		rom.read(buf,4);
		rom.seekg(0x2075E00+*(uint32_t*)buf);
		rom.read(buf,sizeof(buf));
		buf[63]='\0';
		string name=buf;
		// Process the name into a filesystem-friendly format
		for(int n=0;n<name.size();n++)
			switch(name[n]) {
				case '\xE9':
					name[n]='e';
					break;
				case ' ':
					if(n!=(name.size()-1))
						if(('a'<=name[n+1])&&(name[n+1]<='z'))
							name[n+1]-='a'-'A';
				case '(':
				case ')':
				case '!':
				case '.':
				case '\'':
					name.erase(n,1);
					n--;
			}
		// Get the song ID
		rom.seekg(0x176BE2+2*i);
		rom.read(buf,2);
		int id=*(uint16_t*)buf;
		sprintf(buf,"bgm%04u",id);
		string base=buf;
		cout << base << " -> " << name << endl;
		// Make the links
		stringstream cmdSMD;
		stringstream cmdSWD;
		cmdSMD << "ln -s " << argv[2] << '/' << base << ".smd ";
		cmdSWD << "ln -s " << argv[2] << '/' << base << ".swd ";
		cmdSMD << argv[2] << '/' << name << ".smd";
		cmdSWD << argv[2] << '/' << name << ".swd";
		system(cmdSMD.str().c_str());
		system(cmdSWD.str().c_str());
	}
	rom.close();
	return 0;
}
