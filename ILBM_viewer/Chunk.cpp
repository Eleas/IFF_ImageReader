#include "Chunk.h"
#include "utility.h"

IFFReader::CHUNK::CHUNK() : size_(0)
{
}


IFFReader::CHUNK::CHUNK(bytestream& stream) : size_(read_long(stream))
{
}


IFFReader::CHUNK::~CHUNK()
{
}


const uint32_t IFFReader::CHUNK::GetSize() const
{
	return size_;
}


// Stores tag as string. Only implemented for UNKNOWN.
void IFFReader::CHUNK::AddTagLiteral(const string tag)
{
	// ... an identified chunk has a well-known name, so 
	//	we do nothing here (see UNKNOWN chunk).
}
