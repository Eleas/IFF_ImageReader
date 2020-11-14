#pragma once
#include <cstdint>
#include <vector>

using std::vector;

/*
 * Old-style graphics use lots of tricks. The ColorLookup class abstracts  
 * many of those in the form of CLUT (color lookup table) control.
 *
 * In its basic form, CLUT is a reference to a color. Simply put, instead 
 * of storing the absolute color value of a pixel in the data, what's stored 
 * is actually an index, one that points to one of a list of color values -- 
 * the Color Look-Up Table.
 * 
 * This index, being far smaller, means far more compact graphics. On a 16 -
 * color screen, for instance, rather than requiring 24 bits per pixel, each 
 * pixel would be represented by a mere 4 bits, meaning this 16-color image
 * would require 1/6th the amount of memory.
 * 
 * This becomes extra interesting when we take into account the more exotic
 * screen modes the Amiga has to offer, namely colour cycling, EHB, HAM, and
 * copper changes.
 */
namespace IFFReader {
	// Base class for handling lookup tables. Also aims to handle direct 
	// (true-color) lookups, where pixel values equal their exact 
	// representations.
	class ColorLookup
	{
		// Copy of palette used for color lookups.
		vector<uint32_t> colors_;  

		// Whether we use color correction.
		bool color_correction_;

		// Alternate palette, used when color correction is enabled.
		vector<uint32_t> colors_scratch_; 

		// Points to chunkified image data stored in the caller. 
		vector<uint8_t>& data_;

		// Number of bitplanes, used to determine if we correct for OCS.
		uint16_t bitplane_count_;

	public:
		ColorLookup(const vector<uint32_t>& colors, vector<uint8_t>& data, const uint16_t bitplanes);

		// Yields base palette.
		const vector<uint32_t>& GetColors() const;

		// Yields binary contents of image (translated to chunky orientation).
		const vector<uint8_t>& GetData() const;

		// Yields bitplanes of image.
		const int BitplaneCount() const;

		// Looks up a color at the given pixel position.
		virtual const uint32_t at(const size_t index);

		// Toggles whether to use OCS adjusted colors or regular ones.
		void AdjustForOCS(const bool adjust);

		// Shows whether using OCS adjusted colors.
		const bool UsingOCSColorCorrection() const;

		// Test if palette colors is potentially OCS.
		virtual const bool MightBeMangledOCS() const;

		// Test if entire palette has empty low nibbles.
		const bool LowerNibblesZero() const;
	};


	// Extra halfbrite does color in a weird way. It uses 32 colors, and one
	// extra bitplane; the extra bitplane is used for a "shadow" palette,
	// with each color being a repeat of the one in the previous series, 
	// only at halved brightness.
	class ColorLookupEHB : public ColorLookup
	{
	public:
		ColorLookupEHB(const vector<uint32_t>& colors, vector<uint8_t>& data, const uint16_t bitplanes);

		// Looks up a color at the given pixel position.
		const uint32_t at(const size_t index) override;

		// Test if palette colors is potentially OCS.
		const bool MightBeMangledOCS() const override;
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
		ColorLookupHAM(const vector<uint32_t>& colors, vector<uint8_t>& data, const uint16_t bitplanes);

		// Looks up a color at the given pixel position.
		const uint32_t at(const size_t index) override;

		// Test if palette colors is potentially OCS.
		const bool MightBeMangledOCS() const override;
	};
}
