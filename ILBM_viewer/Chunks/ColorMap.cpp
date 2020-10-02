#include "ColorMap.h"
#include "Chunk.h"
#include "utility.h"

IFFReader::CMAP::CMAP()
{
}


// Extracts palette as a collection of colors.
IFFReader::CMAP::CMAP(bytestream& stream) : CHUNK(stream)
{
	const auto color_count{ GetSize() / 3 };	 // 3 bytes: R,G,B.

	// Data is stored on format 0xff gg bb rr.
	for (unsigned int i = 0; i < color_count; ++i) {
		palette_.push_back(
			read_byte(stream) | (read_byte(stream) << 8) | (read_byte(stream) << 16) | (0xff << 24)
		);
	}
}


// Object that handles palette lookups.
const IFFReader::ColorLookup IFFReader::CMAP::GetColors() const
{
	return ColorLookup(palette_);
}


// Object that handles palette lookups.
const IFFReader::ColorLookupEHB IFFReader::CMAP::GetColorsEHB() const
{
	return ColorLookupEHB(palette_);
}


// Object that handles palette lookups.
const IFFReader::ColorLookupHAM IFFReader::CMAP::GetColorsHAM(vector<uint8_t>& data) const
{
	return ColorLookupHAM(palette_, data);
}
