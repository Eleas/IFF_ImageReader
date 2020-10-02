#include "ColorLookup.h"

IFFReader::ColorLookup::ColorLookup()
{
}


// Not const ref due to some kind of pointer magic, simply to demonstrate that there's no
// mutable data being transferred.
IFFReader::ColorLookup::ColorLookup(const vector<uint32_t>& colors) : colors_(colors)
{
}


vector<uint32_t>& IFFReader::ColorLookup::GetColors() 
{ 
	return colors_; 
}


const uint32_t IFFReader::ColorLookup::at(const unsigned int position) 
{
	return colors_.at(position);
}


IFFReader::ColorLookupEHB::ColorLookupEHB() : ColorLookup()
{
}


IFFReader::ColorLookupEHB::ColorLookupEHB(const vector<uint32_t>& colors) : ColorLookup(colors)
{
}


const uint32_t IFFReader::ColorLookupEHB::at(const unsigned int position)
{
	const auto& colors = GetColors();
	return (position < colors.size()) ? colors.at(position) : ((colors.at(position-32) >> 1) | 0xFF000000) & 0xFF7F7F7F; // Halve each color value.
}

IFFReader::ColorLookupHAM::ColorLookupHAM(const vector<uint32_t>& colors, 
	vector<uint8_t>& data) : 
	ColorLookup(colors), 
	data_(data), 
	previous_color_(0)
{
}

// For a HAM image, the at function is different. It checks the 
// two HAM bits, and gets regular color if it's 00. Otherwise, it looks
// at the stored bit in the HAM object.
const uint32_t IFFReader::ColorLookupHAM::at(const unsigned int position)
{
	return uint32_t();
}
