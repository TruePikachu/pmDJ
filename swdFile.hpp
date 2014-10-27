#ifndef __SWDFILE_HPP
#define __SWDFILE_HPP
class swdFile;
class swdFileChunk;
class swdChunkEOD;
class swdChunkKGRP;
class swdChunkPCMD;
class swdChunkPRGI;
class swdChunkWAVI;
#include <fstream>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

class swdFile {
	private:
		std::string	intFilename;
		size_t		pcmdLength;
		size_t		waviLength;
		std::vector< swdFileChunk* >	chunks;
	public:
					 swdFile	(std::ifstream&);
					 swdFile	(const swdFile&);
					~swdFile	();
		swdFile&		operator=	(swdFile);
		friend std::ostream&	operator<<	(std::ostream&,const swdFile&);
		std::string		GetFilename	() const;
		size_t			GetPcmdLength	() const;
		size_t			GetWaviLength	() const;
		int			ChunkCount	() const;
		const swdFileChunk&	operator[]	(int) const;
		const std::vector< swdFileChunk* >& Chunks() const;
};

class swdFileChunk {
	public:
		typedef enum {	UNKNOWN_CHUNK,
				CHUNK_EOD,
				CHUNK_KGRP,
				CHUNK_PCMD,
				CHUNK_PRGI,
				CHUNK_WAVI
		} ChunkType;
	protected:
		char		label[5];
		off_t		chunkOffset;
		size_t		dataSize;
		char*		dataPtr;
		virtual std::ostream&	AdvancedInfo(std::ostream&) const;
	public:
					 swdFileChunk	(std::ifstream&);
					 swdFileChunk	(const swdFileChunk&);
					~swdFileChunk	();
		friend std::ostream&	operator<<	(std::ostream&,const swdFileChunk&);
		swdFileChunk&		operator=	(swdFileChunk);

		std::string		GetLabel	() const;
		size_t			GetSize		() const;
		const char*		GetDataPtr	() const;
		virtual ChunkType	GetType		() const;
		static ChunkType	GetType		(std::ifstream&);
		const swdChunkEOD&	AsEOD		() const;
		const swdChunkKGRP&	AsKGRP		() const;
		const swdChunkPCMD&	AsPCMD		() const;
		const swdChunkPRGI&	AsPRGI		() const;
		const swdChunkWAVI&	AsWAVI		() const;
};

class swdChunkEOD : public swdFileChunk {
	public:
					 swdChunkEOD	(std::ifstream&);
		swdFileChunk::ChunkType	GetType		() const;
};
#endif
