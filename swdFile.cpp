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
		// Align
		off_t sizePadding = (0x10-(file.tellg()%0x10))%0x10;
		char buf[32];
		file.read(buf,sizePadding);
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

swdFileChunk::swdFileChunk(std::ifstream&file) {
	chunkOffset=file.tellg();
	file.read(label,4);
	label[4]='\0';
	file.seekg(chunkOffset+0xC);
	uint32_t buf;
	file.read((char*)&buf,4);
	dataSize=buf;
	dataPtr = new char[buf];
	file.read(dataPtr,buf);
}

swdFileChunk::swdFileChunk(const swdFileChunk&other) {
	for(int i=0;i<4;i++)
		label[i]=other.label[i];
	label[4]='\0';
	chunkOffset=other.chunkOffset;
	dataSize=other.dataSize;
	dataPtr = new char[dataSize];
	memmove(dataPtr,other.dataPtr,dataSize);
}

swdFileChunk::~swdFileChunk() {
	delete dataPtr;
}

std::ostream& operator<<(std::ostream&os,const swdFileChunk&p) {
	os << p.label << ": ";
	char buf[64];
	sprintf(buf,"@0x%08X size 0x%08X",p.chunkOffset,p.dataSize);
	os << buf << endl;
	return os;
}

swdFileChunk& swdFileChunk::operator=(swdFileChunk other) {
	for(int i=0;i<4;i++)
		label[i]=other.label[i];
	label[4]='\0';
	chunkOffset=other.chunkOffset;
	dataSize=other.dataSize;
	dataPtr = new char[dataSize];
	memmove(dataPtr,other.dataPtr,dataSize);
	return *this;
}

std::string swdFileChunk::GetLabel() const {
	return label;
}

size_t swdFileChunk::GetSize() const {
	return dataSize;
}

const char* swdFileChunk::GetDataPtr() const {
	return dataPtr;
}

