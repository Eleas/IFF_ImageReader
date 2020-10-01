#pragma once
#include "Chunk.h"

namespace IFFReader {
	// Represents chunks that are not identified, but still need handling.
	class UNKNOWN : public CHUNK {
		vector<string> unknown_chunk_names_;

	public:
		UNKNOWN();
		UNKNOWN(bytestream& stream);
	};
}
