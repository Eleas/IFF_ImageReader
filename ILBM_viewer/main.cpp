
// O------------------------------------------------------------------------------O
// | Example "Hello World" Program (main.cpp)                                     |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include <fstream>
#include <cstdint>
#include "FileData.h"
#include "olcPixelGameEngine.h"

olc::Pixel GetColor(vector<ILBMReader::color>& palette, uint8_t n) {
	auto& color = palette.at(n);
	return olc::Pixel(color.r, color.b, color.g);
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
		ILBMReader::File fd("..\\ILBM_viewer\\test files\\01A.iff");
		auto palette = fd.GetAsILBM()->GetPalette();
		auto bitfields = fd.GetAsILBM()->GetBitData();

		int size = 8;
		for (int i = 0; i < 32; ++i) {
			FillRect(olc::vi2d((i * size), 0), olc::vi2d((i * size) + size -1, size -1), GetColor(palette, i));
		}
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

