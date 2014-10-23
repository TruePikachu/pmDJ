#ifndef __SWDFILE_HPP
#define __SWDFILE_HPP
class swdFile;
class swdFileChunk;
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
		std::vector< swdFileChunk >	chunks;
	public:
					 swdFile	(std::ifstream&);
		friend std::ostream&	operator<<	(std::ostream&,const swdFile&);
		std::string		GetFilename	() const;
		size_t			GetPcmdLength	() const;
		size_t			GetWaviLength	() const;
		int			ChunkCount	() const;
		const swdFileChunk&	operator[]	(int) const;
		const std::vector< swdFileChunk >& Chunks() const;
};

class swdFileChunk {
	protected:
		char		label[4];
		size_t		dataSize;
		char*		dataPtr;
	public:
					 swdFileChunk	(std::ifstream&);
					 swdFileChunk	(const swdFileChunk&);
					~swdFileChunk	();
		swdFileChunk&		operator=	(swdFileChunk);

		std::string		GetLabel	() const;
		size_t			GetSize		() const;
		const char*		GetDataPtr	() const;
};

#endif
