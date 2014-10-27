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
		swdFileChunk::ChunkType type = swdFileChunk::GetType(file);
		swdFileChunk* newChunk = 0;
		switch(type) {
			case swdFileChunk::CHUNK_EOD:
				newChunk = new swdChunkEOD(file);
				break;
			default:
				newChunk = new swdFileChunk(file);
		}
		chunks.push_back(newChunk);
		// Align
		off_t sizePadding = (0x10-(file.tellg()%0x10))%0x10;
		char buf[32];
		file.read(buf,sizePadding);
	}
	if(file.tellg()!=(myStart+fileLength))
		throw logic_error("Extra bytes???");
}

swdFile::swdFile(const swdFile&other) {
	operator=(other);
}

swdFile::~swdFile() {
	for(vector< swdFileChunk* >::iterator it=chunks.begin();it!=chunks.end();++it)
		delete *it;
}

swdFile& swdFile::operator=(swdFile other) {
	intFilename=other.intFilename;
	pcmdLength=other.pcmdLength;
	waviLength=other.waviLength;
	for(vector< swdFileChunk* >::const_iterator it=other.chunks.begin();it!=other.chunks.end();++it)
		switch((*it)->GetType()) {
			case swdFileChunk::CHUNK_EOD:
				chunks.push_back(new swdChunkEOD((*it)->AsEOD()));
				break;
			default:
				chunks.push_back(new swdFileChunk(**it));
		}
	return *this;
}

std::ostream& operator<<(std::ostream& os, const swdFile& p) {
	os << "Name: " << p.intFilename << endl;
	os << "Chunks: (n=" << p.chunks.size() << ")\n";
	for(vector< swdFileChunk* >::const_iterator it=p.chunks.begin();it!=p.chunks.end();++it)
		os << **it;
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
	return *(chunks[i]);
}

const std::vector< swdFileChunk* >& swdFile::Chunks() const {
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
	sprintf(buf,"@ 0x%08X size 0x%08X",p.chunkOffset,p.dataSize);
	os << buf;
	switch(p.GetType()) {
		case swdFileChunk::CHUNK_EOD:
			os << " (CHUNK_EOD)\n";
			break;
		default:
			os << endl;
	}
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

swdFileChunk::ChunkType swdFileChunk::GetType(std::ifstream& file) {
	char buf[4];
	file.read(buf,4);
	file.seekg(-4,file.cur);
	if(!strncmp(buf,"eod ",4))
		return CHUNK_EOD;
	if(!strncmp(buf,"kgrp",4))
		return CHUNK_KGRP;
	if(!strncmp(buf,"pcmd",4))
		return CHUNK_PCMD;
	if(!strncmp(buf,"prgi",4))
		return CHUNK_PRGI;
	if(!strncmp(buf,"wavi",4))
		return CHUNK_WAVI;
	return UNKNOWN_CHUNK;
}

swdFileChunk::ChunkType swdFileChunk::GetType() const {
	return UNKNOWN_CHUNK;
}

const swdChunkEOD& swdFileChunk::AsEOD() const {
	return *(swdChunkEOD*)this;
}

//////////

swdChunkEOD::swdChunkEOD(std::ifstream& file) : swdFileChunk(file) {
}

swdFileChunk::ChunkType swdChunkEOD::GetType() const {
	return swdFileChunk::CHUNK_EOD;
}
