#pragma once
#include "Chunk.h"

namespace IFFReader {
// Represents a chunk that was not identified, but still needs handling.
class UNKNOWN : public CHUNK {
  vector<string> unknown_chunk_names_;

public:
  UNKNOWN();
  UNKNOWN(bytestream &stream);
};
} // namespace IFFReader
