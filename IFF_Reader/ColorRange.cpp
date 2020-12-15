#include "ColorRange.h"

CRNG::CRNG() {}

CRNG::CRNG(bytestream &stream) : CHUNK(stream) {
	stream.ignore(GetSize()); // Not yet implemented.
}
