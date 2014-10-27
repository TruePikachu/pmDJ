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
			case swdFileChunk::CHUNK_KGRP:
				newChunk = new swdChunkKGRP(file);
				break;
			case swdFileChunk::CHUNK_PCMD:
				newChunk = new swdChunkPCMD(file);
				break;
			case swdFileChunk::CHUNK_PRGI:
				newChunk = new swdChunkPRGI(file);
				break;
			case swdFileChunk::CHUNK_WAVI:
				newChunk = new swdChunkWAVI(file);
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
				chunks.push_back(new swdChunkEOD(*(swdChunkEOD*)*it));
				break;
			case swdFileChunk::CHUNK_KGRP:
				chunks.push_back(new swdChunkKGRP(*(swdChunkKGRP*)*it));
				break;
			case swdFileChunk::CHUNK_PCMD:
				chunks.push_back(new swdChunkPCMD(*(swdChunkPCMD*)*it));
				break;
			case swdFileChunk::CHUNK_PRGI:
				chunks.push_back(new swdChunkPRGI(*(swdChunkPRGI*)*it));
				break;
			case swdFileChunk::CHUNK_WAVI:
				chunks.push_back(new swdChunkWAVI(*(swdChunkWAVI*)*it));
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

std::ostream& swdFileChunk::AdvancedInfo(std::ostream& os) const {
	char b[16];
	for(off_t i=0;i<dataSize;i++) {
		if((i%16)==0) {
			if(i)
				os << endl;
			sprintf(b,"  0x%08X",i);
			os << b;
		}
		if((i%8)==0)
			os << ' ';
		sprintf(b," %02X",(uint8_t)dataPtr[i]);
		os << b;
	}
	return os << endl;
}

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
	os << buf << endl;
	return p.AdvancedInfo(os);
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

//////////

swdChunkEOD::swdChunkEOD(std::ifstream& file) : swdFileChunk(file) {
}

swdFileChunk::ChunkType swdChunkEOD::GetType() const {
	return swdFileChunk::CHUNK_EOD;
}

//////////

swdChunkKGRP::swdChunkKGRP(std::ifstream& file) : swdFileChunk(file) {
}

swdFileChunk::ChunkType swdChunkKGRP::GetType() const {
	return swdFileChunk::CHUNK_KGRP;
}

//////////

swdChunkPCMD::swdChunkPCMD(std::ifstream& file) : swdFileChunk(file) {
}

swdFileChunk::ChunkType swdChunkPCMD::GetType() const {
	return swdFileChunk::CHUNK_PCMD;
}

//////////

swdChunkPRGI::swdChunkPRGI(std::ifstream& file) : swdFileChunk(file) {
}

swdFileChunk::ChunkType swdChunkPRGI::GetType() const {
	return swdFileChunk::CHUNK_PRGI;
}

//////////

swdChunkWAVI::swdChunkWAVI(std::ifstream& file) : swdFileChunk(file) {
}

swdFileChunk::ChunkType swdChunkWAVI::GetType() const {
	return swdFileChunk::CHUNK_WAVI;
}
