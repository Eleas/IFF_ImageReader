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


const uint32_t IFFReader::ColorLookup::at(const unsigned int index) 
{
	return colors_.at(index);
}


IFFReader::ColorLookupEHB::ColorLookupEHB() : ColorLookup()
{
}


IFFReader::ColorLookupEHB::ColorLookupEHB(const vector<uint32_t>& colors) : ColorLookup(colors)
{
}


const uint32_t IFFReader::ColorLookupEHB::at(const unsigned int index)
{
	const auto& colors = GetColors();
	return (index < colors.size()) ? colors.at(index) : ((colors.at(index-32) >> 1) | 0xFF000000) & 0xFF7F7F7F; // Halve each color value.
}


IFFReader::ColorLookupHAM::ColorLookupHAM(const vector<uint32_t>& colors, 
	vector<uint8_t>& data) : 
	ColorLookup(colors), 
	data_(data), 
	previous_color_(0)
{
}


// For a HAM image, the at method is different. It checks the 
// two HAM bits, and gets regular color if it's 00. Otherwise, it looks
// at the previous colors, and alters red, green or blue by the four first
// bits.
const uint32_t IFFReader::ColorLookupHAM::at(const unsigned int index)
{
	const auto given_value = data_.at(index);
	const auto change = (given_value & 0xf);

	switch ( given_value >> 4 ) {
	case 0:
		previous_color_ = GetColors().at(given_value);
		break;
	case 1: // Red
		previous_color_ = ((previous_color_ & 0xffffff00) | change);
		break;
	case 2: // Green
		previous_color_ = ((previous_color_ & 0xffff00ff) | (change << 8));
		break;
	case 3: // Blue
		previous_color_ = ((previous_color_ & 0xff00ffff) | (change << 16));
		break;
	}

	return previous_color_;
}
