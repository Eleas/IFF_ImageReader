#pragma once
#include "utility.h"
#include "Chunk.h"
#include "ColorLookup.h"

namespace IFFReader {

	class CMAP : public CHUNK {
		vector<uint32_t> alt_palette_;

	public:
		CMAP();
		CMAP(bytestream& stream);

		// Extracts palette from raw data.
		const ColorLookup GetColors() const;
	};
}
