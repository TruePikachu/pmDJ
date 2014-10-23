#include "swdFile.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>
using namespace std;

swdFile::swdFile(std::ifstream& file) {
	off_t myStart = file.tellg();
	{ // Check magic number
		char buf[4];
		file.read(buf,4);
		if(strncmp(buf,"swdl",4))
			throw runtime_error("Doesn't look like a .swd");
	}
	{ // Get filename
		char buf[17];
		file.seekg(myStart+0x20);
		file.read(buf,16);
		buf[16]='\0';
		intFilename = buf;
	}
	{ // Get chunk lengths
		uint32_t buf;
		file.seekg(myStart+0x40);
		file.read((char*)&buf,4);
		pcmdLength=buf;
		file.seekg(myStart+0x4C);
		file.read((char*)&buf,4);
		waviLength=buf;
	}
	size_t fileLength;
	{ // Get file length
		uint32_t buf;
		file.seekg(myStart+0x08);
		file.read((char*)&buf,4);
		fileLength=buf;
	}
	file.seekg(myStart+0x50);
	while(file.tellg()<(myStart+fileLength)) {
		swdFileChunk newChunk(file);
		chunks.push_back(newChunk);
	}
	if(file.tellg()!=(myStart+fileLength))
		throw logic_error("Extra bytes???");
}

std::ostream& operator<<(std::ostream& os, const swdFile& p) {
	os << "Name: " << p.intFilename << endl;
	os << "Chunks: (n=" << p.chunks.size() << ")\n";
	for(vector< swdFileChunk >::const_iterator it=p.chunks.begin();it!=p.chunks.end();++it)
		os << *it;
	return os;
}

std::string swdFile::GetFilename() const {
	return intFilename;
}

size_t swdFile::GetPcmdLength() const {
	return pcmdLength;
}

size_t swdFile::GetWaviLength() const {
	return waviLength;
}

int swdFile::ChunkCount() const {
	return chunks.size();
}

const swdFileChunk& swdFile::operator[](int i) const {
	return chunks[i];
}

const std::vector< swdFileChunk >& swdFile::Chunks() const {
	return chunks;
}

//////////


