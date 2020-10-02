#pragma once
#include "utility.h"
#include "Chunk.h"
#include "ColorLookup.h"

namespace IFFReader {

	class CMAP : public CHUNK {
		vector<uint32_t> palette_;

	public:
		CMAP();
		CMAP(bytestream& stream);

		// Extracts palette from raw data.
		const ColorLookup GetColors() const;
		const ColorLookupEHB GetColorsEHB() const;
		const ColorLookupHAM GetColorsHAM(vector<uint8_t>& data) const;
	};
}
