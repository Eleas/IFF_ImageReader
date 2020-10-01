#include "utility.h"

// Reads the ASCII tag name (always four bytes).
const string IFFReader::read_tag(bytestream& stream)
{
	char temptag[4];
	stream.read(reinterpret_cast<uint8_t*>(temptag), 4);
	return string(temptag, sizeof(temptag) / sizeof(char));
}


// Reads big endian longword (4 bytes), returns little endian.
const uint32_t IFFReader::read_long(bytestream& stream)
{
	uint32_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 4);
	uint32_t tmp = ((buffer << 8) & 0xFF00FF00) | ((buffer >> 8) & 0xFF00FF);
	buffer = (tmp << 16) | (tmp >> 16);
	return buffer;
}


// Reads big endian word (2 bytes), returns little endian.
const uint16_t IFFReader::read_word(bytestream& stream)
{
	uint16_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 2);
	return (buffer >> 8) | (buffer << 8);
}


// Reads byte.
const uint8_t IFFReader::read_byte(bytestream& stream)
{
	uint8_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 1);
	return buffer;
}
