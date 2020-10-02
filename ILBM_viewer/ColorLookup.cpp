#include "ColorLookup.h"

IFFReader::ColorLookup::ColorLookup()
{
}


IFFReader::ColorLookup::ColorLookup(const vector<uint32_t>& colors) : colors_(colors)
{
}


// For EHB object, just put this in constructor.
void IFFReader::ColorLookup::TreatAsEHB()
{
	const size_t sz = colors_.size();
	for (int i = 0; i < sz; ++i) {
		colors_.push_back(((colors_.at(i) >> 1) | 0xFF000000) & 0xFF7F7F7F); // Halve each color value.
	}
}


// For a HAM image, the at function is different. It checks the 
// two HAM bits, and gets regular color if it's 00. Otherwise, it looks
// at the stored bit in the HAM object.
const uint32_t IFFReader::ColorLookup::at(const unsigned int position) const
{
	return colors_.at(position);
}
