#include "Chunk.h"
#include "utility.h"


IFFReader::CHUNK::CHUNK() : size_(0)
{
}


IFFReader::CHUNK::CHUNK(bytestream& stream) : 
	size_(read_long(stream))
{
}


IFFReader::CHUNK::~CHUNK()
{
}


const uint32_t IFFReader::CHUNK::GetSize() const
{
	return size_;
}
