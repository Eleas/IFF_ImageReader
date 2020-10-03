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
	bool potentially_ocs = true;				 // Check for lower-order nibble in color bytes

	// We store our colordata on the format 0xff gg bb rr.
	for (unsigned int i = 0; i < color_count; ++i) {
		palette_.push_back(
			read_byte(stream) | (read_byte(stream) << 8) | (read_byte(stream) << 16) | (0xff << 24));
		if ((palette_.back() & 0x00f0f0f0) != 0) {  
			potentially_ocs = true;
		}
	}

	if (!potentially_ocs) {
		return;
	}

	for (auto& p : palette_) {
		p |= ((p&0x00ffffff) >> 4); // Duplicate each color nibble into the low nibble.
	}
}


// Object that handles palette lookups.
const IFFReader::ColorLookup IFFReader::CMAP::GetColors(vector<uint8_t>& data) const
{
	return ColorLookup(palette_, data);
}


// Object that handles palette lookups.
const IFFReader::ColorLookupEHB IFFReader::CMAP::GetColorsEHB(vector<uint8_t>& data) const
{
	return ColorLookupEHB(palette_, data);
}


// Object that handles palette lookups.
const IFFReader::ColorLookupHAM IFFReader::CMAP::GetColorsHAM(vector<uint8_t>& data) const
{
	return ColorLookupHAM(palette_, data);
}
