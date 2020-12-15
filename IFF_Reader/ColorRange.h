#pragma once
#include "Chunk.h"

class CRNG : public IFFReader::CHUNK {

public:
  CRNG();
  CRNG(bytestream &stream);
};
