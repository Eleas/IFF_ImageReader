#include "ColorLookup.h"


// Not const ref due to some kind of pointer magic, simply to convey that no 
// mutable data is transferred.
IFFReader::ColorLookup::ColorLookup(const vector<uint32_t>& colors, 
	const vector<uint8_t>& data) :
	colors_(colors), data_(data)
{
}


// Yields the base palette.
const vector<uint32_t>& IFFReader::ColorLookup::GetColors() const
{ 
	return colors_; 
}


// Yields the image binary contents (translated into chunky orientation).
const vector<uint8_t>& IFFReader::ColorLookup::GetData() const
{
	return data_;
}


// Looks up a color at the given pixel position.
const uint32_t IFFReader::ColorLookup::at(const size_t index)
{
	return colors_.at(GetData().at(index));
}


IFFReader::ColorLookupEHB::ColorLookupEHB(const vector<uint32_t>& colors, 
	vector<uint8_t>& data) : 
	ColorLookup(colors, data)
{
}


// Looks up a color at the given pixel position.
const uint32_t IFFReader::ColorLookupEHB::at(const size_t index)
{
	const auto value = GetData().at(index);
	const auto& colors = GetColors();

	// For colors 32-63, alve each regular color value.
	return (value < colors.size()) ? 
		colors.at(value) : 
		((colors.at(value -32) >> 1) | 0xFF000000) & 0xFF777777; 
}


IFFReader::ColorLookupHAM::ColorLookupHAM(const vector<uint32_t>& colors, 
	vector<uint8_t>& data) : 
	ColorLookup(colors, data), 
	previous_color_(0)
{
}


// For a HAM image, the at method is different. It checks the 
// two HAM bits, and gets regular color if it's 00. Otherwise, it looks
// at the previous colors, and alters red, green or blue by the four first
// bits.
const uint32_t IFFReader::ColorLookupHAM::at(const size_t index)
{
	const auto given_value = GetData().at(index);
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
