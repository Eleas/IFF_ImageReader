
// O------------------------------------------------------------------------------O
// | Example "Hello World" Program (main.cpp)                                     |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include <fstream>
#include <cstdint>
#include "FileData.h"
#include "olcPixelGameEngine.h"

olc::Pixel GetColor(vector<ILBMReader::color>& palette, uint8_t n) 
{
	auto& color = palette.at(n);
	return olc::Pixel(color.r, color.b, color.g);
}

const unsigned int GetByteOffset(const unsigned int x, const unsigned int y, const vector<uint8_t>& data, const unsigned int bitplane, const unsigned int width) {
	auto bytebound = width / 8;
	return (x/bytebound) + (y * bytebound) + (bitplane * bytebound);
}

const bool GetBitInByte(uint8_t byte, uint8_t n) {
	return byte &(1 << n);
}



// Override base class with your custom functionality
class Viewer : public olc::PixelGameEngine
{
public:
	Viewer()
	{
		// Name your application
		sAppName = "ILBM viewer";
	}
	double cyclic = 0;

public:
	bool OnUserCreate() override
	{
		ILBMReader::File fd("..\\ILBM_viewer\\test files\\stored_ac.iff");

		// All these should be hidden later; the only thing we expose is the end result data.
		auto palette = fd.GetAsILBM()->GetPalette();
		auto bitfield = fd.GetAsILBM()->GetData();
		auto header = fd.GetAsILBM()->GetHeader();



		// Next, function in GetAsILBM() should give us a vector of bytes for a given byte.
		// Each of those are stacked and used for lookup.

		// Called once at the start, so create things here
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		return true;
	}
};


int main()
{
	Viewer ilbm_viewer;
	if (ilbm_viewer.Construct(256, 240, 2, 2, false, true))
		ilbm_viewer.Start();
	return 0;
}

