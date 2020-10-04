
// O------------------------------------------------------------------------------O
// | IFF Reader										                              |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include <memory>
#include "FileData.h"
#include "lyra/lyra.hpp"
#include "olcPixelGameEngine.h"

using std::unique_ptr;

// Override base class with your custom functionality
class Viewer : public olc::PixelGameEngine
{
	unique_ptr<IFFReader::File> iff_file_;
	shared_ptr<IFFReader::ILBM> ilbm_;

public:
	
	Viewer()
	{
		// Name your application
		sAppName = "IFF reader";
	}
	double cyclic = 0;

public:
	// Called once at the start, so create things here
	bool OnUserCreate() override
	{
		// To do:
		// * Recognize and parse HAM and EHB when missing CAMG.

		// Proposed syntax for IFF error handling.
		//
		// iff_file_ = unique_ptr<IFFReader::File> (new IFFReader::File(" ... " ));
		// if (!iff_file_->valid()) { 
		//		std::cout << iff_file_.errors();
		//		return;
		// }
		// 
		// ilbm_ = std::make_shared<IFFReader::ILBM>(*iff_file_->AsILBM());
		// if (!ilbm_->valid()) {
		//		std::cout << ilbm_.errors();
		//		return;
		// }

		iff_file_ = unique_ptr<IFFReader::File> 
			(new IFFReader::File("C:\\Users\\Björn\\source\\C++ projects\\IFF_ImageReader\\ILBM_viewer\\test files\\ham_image.iff"));
		if (iff_file_->GetType() == IFFReader::IFF_T::FORM_NOT_FOUND) {
			std::cout << "No valid IFF file found. Have you checked the file path?\n";
			return false;
		}
		if (iff_file_->GetType() == IFFReader::IFF_T::UNREADABLE) {
			std::cout << "IFF file mangled; cannot open.\n";
			return false;
		}
		if (iff_file_->GetType() == IFFReader::IFF_T::UNKNOWN_FORMAT) {
			std::cout << "This ILBM has an unsupported format.\n";
			return false;
		}

		ilbm_ = std::make_shared<IFFReader::ILBM>(*iff_file_->AsILBM());
		SetScreenSize(ilbm_->width(), ilbm_->height());

		return true;
	}


	bool OnUserUpdate(float fElapsedTime) override
	{
		if (ilbm_.get() == nullptr) {
			return true;
		}
		for (unsigned int y = 0; y < ilbm_->height(); ++y) {
			for (unsigned int x = 0; x < ilbm_->width(); ++x) {
				auto px = ilbm_->color_at(x, y);
				Draw(x, y, olc::Pixel(px));
			}
		}

		return true;
	}
};


int main(int argc, char* argv[])
{
	Viewer ilbm_viewer;
	if (ilbm_viewer.Construct(320, 240, 2, 2, false, true)) {
		ilbm_viewer.Start();
		// Supply args for image folder to view.
	}
	return 0;
}

