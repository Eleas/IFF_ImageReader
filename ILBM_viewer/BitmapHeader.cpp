#include "BitmapHeader.h"

IFFReader::BMHD::BMHD() : width_{ 0 }, height_{ 0 }, xcoordinate_{ 0 },
ycoordinate_{ 0 }, bitplanes_{ 0 }, masking_{ 0 }, compression_{ 0 },
transparency_{ 0 }, x_aspect_ratio_{ 0 }, y_aspect_ratio_{ 0 },
page_width_{ 0 }, page_height_{ 0 }
{
}


IFFReader::BMHD::BMHD(bytestream& stream) : CHUNK(stream)
{
	width_ = read_word(stream);
	height_ = read_word(stream);
	xcoordinate_ = read_word(stream);
	ycoordinate_ = read_word(stream);

	bitplanes_ = read_byte(stream);
	masking_ = read_byte(stream);
	compression_ = read_byte(stream);
	stream.ignore(1); // 1 byte padding
	transparency_ = read_word(stream);

	x_aspect_ratio_ = read_byte(stream);
	y_aspect_ratio_ = read_byte(stream);

	page_width_ = read_word(stream);
	page_height_ = read_word(stream);
}


const uint16_t IFFReader::BMHD::GetWidth() const
{
	return width_;
}


const uint16_t IFFReader::BMHD::GetHeight() const
{
	return height_;
}


const uint16_t IFFReader::BMHD::GetBitplanesCount() const
{
	return bitplanes_;
}


const uint8_t IFFReader::BMHD::Compression() const
{
	return compression_;
}

