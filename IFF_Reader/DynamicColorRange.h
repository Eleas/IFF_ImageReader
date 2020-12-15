#pragma once
#include "Chunk.h"
class DRNG : public IFFReader::CHUNK {
public:
  DRNG();
  DRNG(bytestream &stream);
};
