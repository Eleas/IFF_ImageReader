#pragma once
#include <cstdint>
#include <vector>

using std::vector;

/*
 * Old-style graphics use lots of tricks. The ColorLookup classes handle one 
 * of them, the CLUT table. Simply put, instead of storing the absolute
 * color value of a pixel in the data, what's stored is actually an index,
 * one that points to one of a list of color values -- the Color Look-Up Table.
 * 
 * This index, being far smaller, means far more compact graphics. On a 16 -
 * color screen, for instance, rather than requiring 24 bits per pixel, each 
 * pixel would be represented by a mere 4 bits, meaning this 16-color image
 * would require 1/6th the amount of memory.
 * 
 */
namespace IFFReader {
	// Base class for handling lookup tables. Also handles direct (true-color)
	// lookups, where pixel values equal their exact representations.
	// 
	// This should not be used that way, because it implies a total of 8 bits
	// color. Instead, reconsider how we want to use lookup method for 
	// true-color.
	class ColorLookup
	{
		vector<uint32_t> colors_;  // A copy of the palette.
		vector<uint8_t>& data_;	   // A *reference* to chunkified image data. 

	public:
		ColorLookup(const vector<uint32_t>& colors, vector<uint8_t>& data);

		// Yields base palette.
		const vector<uint32_t>& GetColors() const;

		// Yields binary contents of image (translated to chunky orientation).
		const vector<uint8_t>& GetData() const;

		// Looks up a color at the given pixel position.
		virtual const uint32_t at(const size_t index);
	};


	// Extra halfbrite does color in a weird way. It uses 32 colors, and one
	// extra bitplane; the extra bitplane is used for a "shadow" palette,
	// with each color being a repeat of the one in the previous series, 
	// only at halved brightness.
	class ColorLookupEHB : public ColorLookup
	{
	public:
		ColorLookupEHB(const vector<uint32_t>& colors, vector<uint8_t>& data);

		// Looks up a color at the given pixel position.
		const uint32_t at(const size_t index) override;
	};


	// HAM, or Hold-And-Modify, is another impressive trick. The full
	// description is involved, but essentially it uses two of its 
	// 6 bitplanes to indicate that this value is actually a color 
	// change to the previous ("held") color. The result is potentially
	// photorealistic, but with certain inherent limitations.
	class ColorLookupHAM : public ColorLookup
	{
		uint32_t previous_color_;
	public:
		ColorLookupHAM(const vector<uint32_t>& colors, vector<uint8_t>& data);

		// Looks up a color at the given pixel position.
		const uint32_t at(const size_t index) override;
	};
}
