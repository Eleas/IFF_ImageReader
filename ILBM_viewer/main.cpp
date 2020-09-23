
// O------------------------------------------------------------------------------O
// | IFF Reader										                              |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include <fstream>
#include <cstdint>
#include "FileData.h"
#include "olcPixelGameEngine.h"


// Override base class with your custom functionality
class Viewer : public olc::PixelGameEngine
{
public:
	Viewer()
	{
		// Name your application
		sAppName = "IFF reader";
	}
	double cyclic = 0;

public:
	bool OnUserCreate() override
	{
		// To do:
		// * Recognize and parse EHB.
		// * Recognize and parse HAM (whether CAMG chunk or not).
		// * Break out Palette Matching step as object.

		IFFReader::File fd("C:\\Users\\Björn\\source\\C++ projects\\IFF_ImageReader\\ILBM_viewer\\test files\\00A.iff");
		if (fd.GetType() == IFFReader::IFF_T::FORM_NOT_FOUND) {
			std::cout << "No valid IFF file found. Have you checked the file path?\n";
		}
		if (fd.GetType() == IFFReader::IFF_T::UNREADABLE) {
			std::cout << "IFF file mangled; cannot open.\n";
			return true;
		}
		if (fd.GetType() == IFFReader::IFF_T::UNKNOWN_FORMAT) {
			std::cout << "This ILBM has an unsupported format.\n";
			return true;
		}

		auto iff_image = fd.AsILBM();

		SetScreenSize(iff_image->width(), iff_image->height());

		for (auto& px : *iff_image) {
			Draw(px.x, px.y, olc::Pixel(px.r, px.g, px.b));
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
	if (ilbm_viewer.Construct(320, 240,2,2, false, true))
		ilbm_viewer.Start();
	return 0;
}

