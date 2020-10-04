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

		// Corrects missing color information (use for potential OCS IFF images only)
		void CorrectOCSBrightness();

		// Extracts palette from raw data.
		const ColorLookup GetColors(vector<uint8_t>& data) const;
		const ColorLookupEHB GetColorsEHB(vector<uint8_t>& data) const;
		const ColorLookupHAM GetColorsHAM(vector<uint8_t>& data) const;
	};
}
