
// O------------------------------------------------------------------------------O
// | Example "Hello World" Program (main.cpp)                                     |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

//const double PI = 2 * acos(0.0); // Simple way to do it.

// Let's just do this light and breezy.

// Sixth, identify unpacked bitfield data. Print every pixel row, line by line.
// Seventh, see if we can unpack in some way.

#include <fstream>
#include <cstdint>
#include "FileData.h"



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
		//std::basic_ifstream<std::uint8_t> fd("..\\ILBM_viewer\\test files\\01A.iff", std::ios::binary);
		IFFImageReader::FileData fd("..\\ILBM_viewer\\test files\\01A.iff");

		//IFFImageReader::FORM ilbm(fd);
		// To read this file, we open it.



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

