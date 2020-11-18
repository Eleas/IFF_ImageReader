#include "Unknown.h"

// Represents a tag not recognized.
IFFReader::UNKNOWN::UNKNOWN() {}

// Unknown tags are bypassed without extracting data.
IFFReader::UNKNOWN::UNKNOWN(bytestream &stream) : CHUNK(stream) {
  stream.ignore(GetSize()); // Sod off, on to next chunk with ye.
}
