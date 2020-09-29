
// O------------------------------------------------------------------------------O
// | IFF Reader										                              |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include <exception>
#include <fstream>
#include <memory>
#include <cstdint>
#include "FileData.h"
#include "olcPixelGameEngine.h"
#include "iff_exception.h"

using std::exception;

// Override base class with your custom functionality
class Viewer : public olc::PixelGameEngine
{
	shared_ptr<IFFReader::File> file; // Just one file for now.

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

		try {
			file = std::shared_ptr<IFFReader::File>(new IFFReader::File ("C:\\Users\\Björn\\source\\C++ projects\\IFF_ImageReader\\ILBM_viewer\\test files\\01A.iff"));
		}
		catch (iff_bad_or_mangled_error& e) {
			std::cout << e.what();
		}
		catch (exception e) {
			std::cout << "Something entirely unexpected happened: " << e.what() << ".";
		}

		auto iff_image = file->AsILBM();

		SetScreenSize(iff_image->width(), iff_image->height());

		//
		//for (auto& px : *iff_image) {
		//	Draw(px.x, px.y, olc::Pixel(px.r, px.g, px.b));
		//}
		//
		// Previous should be removed, in favor of simpler structures.

		for (unsigned int y = 0; y < iff_image->height(); ++y) {
			for (unsigned int x = 0; x < iff_image->width(); ++x) {
				auto px = iff_image->at(x, y);
				Draw(x, y, olc::Pixel(px.r, px.g, px.b));
			}
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

