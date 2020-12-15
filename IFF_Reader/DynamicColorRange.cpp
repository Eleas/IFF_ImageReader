#include "DynamicColorRange.h"

DRNG::DRNG() {}

DRNG::DRNG(bytestream &stream) : CHUNK(stream) {
  stream.ignore(GetSize()); // Not yet implemented.
}
