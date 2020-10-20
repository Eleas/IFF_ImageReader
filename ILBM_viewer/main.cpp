
// O------------------------------------------------------------------------------O
// | IFF Reader										                              |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include <memory>
#include "FileData.h"
#include "lyra/lyra.hpp"
#include "olcPixelGameEngine.h"
#include <filesystem>

using std::unique_ptr;

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
//
// Return values via Enum: NoError, FileNotFound, FileNotIFF, FormatNotSupported, ParseErrorHead, ParseErrorBody
// 


class IFF_ILBM {
	unique_ptr<IFFReader::File> file;
	shared_ptr<IFFReader::ILBM> ilbm;

public:
	IFF_ILBM(const string& path) {
		file = unique_ptr<IFFReader::File>
			(new IFFReader::File(path));

		using IFFReader::IFF_ERRCODE;

		switch (file->GetError()) {
		case IFF_ERRCODE::FILE_NOT_FOUND:			std::cout << "No valid IFF file found. Have you checked the file path?\n";	return;
		case IFF_ERRCODE::COULD_NOT_PARSE_AS_IFF:   std::cout << "IFF file mangled; cannot open.\n"; return;
		case IFF_ERRCODE::COULD_NOT_PARSE_HEAD:		std::cout << "IFF file mangled; failed to parse head.\n"; return;
		case IFF_ERRCODE::COULD_NOT_PARSE_BODY:		std::cout << "IFF file mangled; failed to parse body.\n"; return;
		default:
			if (file->GetType() == IFFReader::IFF_T::UNKNOWN_FORMAT) {
				std::cout << "This ILBM has an as-yet unsupported format.\n";
				return;
			}
		}

		ilbm = std::make_shared<IFFReader::ILBM>(*file->AsILBM());
	}
	IFF_ILBM(){}

	shared_ptr<IFFReader::ILBM> Get() { 
		return ilbm; 
	}
};


class Renderer : public olc::PixelGameEngine
{
	vector<IFF_ILBM> images_;
	size_t current_image = 0;


	void AddImage(IFF_ILBM& img) {
		images_.emplace_back(std::move(img));
	}

public:
	
	Renderer()
	{
		sAppName = "IFF reader";
	}
	double cyclic = 0;

public:
	// Called once at the start, so create things here
	bool OnUserCreate() override
	{
		// Select among the images already established.
		const auto& this_image = images_.at(current_image).Get();
		const auto image_count = images_.size();

		// Write pixels
		for (unsigned int y = 0; y < this_image->height(); ++y) {
			for (unsigned int x = 0; x < this_image->width(); ++x) {
				auto px = this_image->color_at(x, y);
			}
		}

		return true;
	}


	bool OnUserUpdate(float fElapsedTime) override
	{
		const auto& this_image = images_.at(current_image).Get();
		const auto image_count = images_.size();

		if (this_image->width() != ScreenWidth() || this_image->height() != ScreenHeight()) {
			SetScreenSize(this_image->width(), this_image->height());
		}

		for (unsigned int y = 0; y < this_image->height(); ++y) {
			for (unsigned int x = 0; x < this_image->width(); ++x) {
				auto px = this_image->color_at(x, y);
				Draw(x, y, olc::Pixel(px));
			}
		}

		if (images_.size() > 1 && GetKey(olc::Key::LEFT).bReleased) {
			--current_image;
			Clear(olc::BLACK);
		}

		if (images_.size() > 1 &&
			(GetKey(olc::Key::RIGHT).bReleased ||
				GetKey(olc::Key::SPACE).bReleased)) {
			++current_image;
			Clear(olc::BLACK);
		}

		// Wrap around.
		if (current_image >= image_count) {
			current_image -= image_count;
		}
		else if (current_image < 0) {
			current_image += image_count;
		}

		const auto exit_key_pressed = GetKey(olc::Key::ESCAPE).bPressed ||
			GetKey(olc::Key::ENTER).bPressed || GetKey(olc::Key::Q).bPressed;

		return (!exit_key_pressed);	// Close viewer on keypress.
	}

	const bool Viewable() const {
		return images_.size() != 0;
	}

	const bool AddImages(const std::vector<std::filesystem::path>& paths) {
		// Add files to collection (=open them for viewing).
		bool file_added = false;
		for (auto& f : paths) {
			auto image = IFF_ILBM(f.string());
			if (image.Get()) {
				AddImage(image);
				file_added = true;
			}
		}
		return file_added;
	}
};



// Encode a concept of what the system should do. 
// An enum for the desired task.

namespace fs = std::filesystem;


const bool CheckPath(const string path) {
	auto temp_path = path;
	// Patch for powershell bug
	if (!temp_path.empty() && temp_path.back() == '"') {
		temp_path.pop_back();
	}
	const auto abspath = fs::absolute(temp_path);

	if (fs::exists(abspath.string())) { 
		return true;
	}

	std::cout << "File or path " << abspath.string() << " not found.\n";
	return false;
}


// To do next:
// create an output file, binary, just contains the color values supposed to be drawn.
// Name it and put it in a testing folder.


int main(int argc, char* argv[])
{
	Renderer ilbm_viewer;
	
	string path;
	auto generate_testing_data = false;
	auto show_help = false;
	auto cli = lyra::cli_parser()
		| lyra::help(show_help)
		| lyra::opt(generate_testing_data)["-g"]["--gentest"]("Generate testing data.")
		| lyra::arg(path, "path")("File or folder to view.");

	auto result = cli.parse({ argc, argv });

	if (path.empty()) {
		std::cout << "You need to supply a file path.\n";
		return 1;
	}

	if (!CheckPath(path)) {
		return 1; 
	}

	const auto abspath = fs::absolute(path);

	// This is actually what we want to do.
	std::vector<fs::path> file_paths;

	// Get file candidates.
	if (fs::is_regular_file(abspath)) {
		file_paths.push_back(abspath.string());
	}
	else if (fs::is_directory(abspath)) {
		// Step through each file, ensure that they're at least
		// IFF files; if they are, add them to list.
		for (auto& f : fs::directory_iterator(abspath)) {
			file_paths.push_back(f.path());
		}
	}


	if (file_paths.size() == 0) {
		std::cout << "No file found in folder.\n";
		return 1;
	}


	// Add files to collection (=open them for viewing).
	if (!ilbm_viewer.AddImages(file_paths) && ilbm_viewer.Viewable()) {
		std::cout << "No suitable IFF files found in folder.\n";
		return 1;
	}

	// Now to test whether or not we are generating test files.




	if (ilbm_viewer.Construct(320, 240, 2, 2, false, true)) {
		ilbm_viewer.Start();
		// Supply args for image folder to view.
	}
	return 0;
}

