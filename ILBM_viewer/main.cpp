
// O------------------------------------------------------------------------------O
// | IFF Reader										                              |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include <memory>
#include "FileData.h"
#include "lyra/lyra.hpp"
#include "olcPixelGameEngine.h"
#include <filesystem>

using std::cout;
using std::ios;
using std::make_shared;
using std::move;
using std::ofstream;
using std::unique_ptr;
namespace fs = std::filesystem;

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
	fs::path filepath;
	unique_ptr<IFFReader::File> file;
	shared_ptr<IFFReader::ILBM> ilbm;

public:
	IFF_ILBM(const fs::path& path) : filepath(path) {
		file = unique_ptr<IFFReader::File>
			(new IFFReader::File(path.string()));

		using IFFReader::IFF_ERRCODE;

		switch (file->GetError()) {
		case IFF_ERRCODE::FILE_NOT_FOUND:			cout << "No valid IFF file found. Have you checked the file path?\n";	return;
		case IFF_ERRCODE::COULD_NOT_PARSE_AS_IFF:   cout << "IFF file mangled; cannot open.\n"; return;
		case IFF_ERRCODE::COULD_NOT_PARSE_HEAD:		cout << "IFF file mangled; failed to parse head.\n"; return;
		case IFF_ERRCODE::COULD_NOT_PARSE_BODY:		cout << "IFF file mangled; failed to parse body.\n"; return;
		default:
			if (file->GetType() == IFFReader::IFF_T::UNKNOWN_FORMAT) {
				cout << "This ILBM has an as-yet unsupported format.\n";
				return;
			}
		}

		ilbm = make_shared<IFFReader::ILBM>(*file->AsILBM());
	}

	IFF_ILBM(){}

	shared_ptr<IFFReader::ILBM> Get() const { 
		return ilbm; 
	}

	const bool IsPath(const fs::path path) const { 
		return path == filepath;
	};
};


class Renderer : public olc::PixelGameEngine
{
	vector<IFF_ILBM> images_;
	size_t current_image = 0;

	void AddImage(IFF_ILBM& img) {
		images_.emplace_back(move(img));
	}

public:
	
	Renderer()
	{
		sAppName = "IFF reader";
	}
	double cyclic = 0;

public:
	const vector<uint32_t> GetData(const size_t n) const {
		vector<uint32_t> contents;
		auto data = images_.at(n).Get();
		for (unsigned int y = 0; y < data->height(); ++y) {
			for (unsigned int x = 0; x < data->width(); ++x) {
				auto px = data->color_at(x, y);
				contents.push_back(px);
			}
		}
		return contents;
	}


	const size_t GetFileByAbspath(const fs::path path) const {
		for (size_t i = 0; i < images_.size(); ++i) {
			if (images_.at(i).IsPath(path)) {
				return i;
			}
		}
		return images_.size();
	}


	const bool CompareData(const size_t n, const vector<uint32_t> other) const {
		vector<uint32_t> contents;
		auto data = images_.at(n).Get();
		size_t o = 0;
		for (unsigned int y = 0; y < data->height(); ++y) {
			for (unsigned int x = 0; x < data->width(); ++x) {
				auto px = data->color_at(x, y);
				if (other.at(o++) != px) {
					return false;
				}
			}
		}
		return true;
	}


	void DisplayImage() 
	{
		// Select among the images already established.
		const auto& this_image = images_.at(current_image).Get();
		if (this_image->width() != ScreenWidth() || this_image->height() != ScreenHeight()) {
			SetScreenSize(this_image->width(), this_image->height());
		}

		// Write pixels
		for (unsigned int y = 0; y < this_image->height(); ++y) {
			for (unsigned int x = 0; x < this_image->width(); ++x) {
				auto px = this_image->color_at(x, y);
				Draw(x, y, olc::Pixel(px));
			}
		}
	}

	// Called once at the start, so create things here
	bool OnUserCreate() override 
	{
		DisplayImage();
		return true;
	}


	bool OnUserUpdate(float fElapsedTime) override
	{
		const auto image_count = images_.size();
		DisplayImage();

		if (images_.size() > 1 && 
			GetKey(olc::Key::LEFT).bReleased) {
			--current_image;
			Clear(olc::BLACK);
		}

		if (images_.size() > 1 &&
			(GetKey(olc::Key::RIGHT).bReleased ||
			 GetKey(olc::Key::SPACE).bReleased )) {
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

		const auto exit_key_pressed = 
			GetKey(olc::Key::ESCAPE).bPressed ||
			GetKey(olc::Key::ENTER).bPressed || 
			GetKey(olc::Key::Q).bPressed ||
			GetKey(olc::Key::X).bPressed;

		return (!exit_key_pressed);	// Close viewer on keypress.
	}

	const bool Viewable() const 
	{
		return images_.size() != 0;
	}

	const bool AddImages(const vector<fs::path>& paths) 
	{   // Add files to collection (=open them for viewing).
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


const bool CheckPath(const string path) 
{ 	// Patch for powershell bug
	auto temp_path = path;
	if (!temp_path.empty() && temp_path.back() == '"') {
		temp_path.pop_back();
	}
	const auto abspath = fs::absolute(temp_path);

	if (fs::exists(abspath.string())) { 
		return true;
	}

	cout << "File or path " << abspath.string() << " not found.\n";
	return false;
}


const vector<fs::path>  GetPathsInFolder(const fs::path & path) 
{	// We now have the folder path.
	vector<fs::path> file_paths;

	// Get file candidates.
	if (fs::is_regular_file(path)) {
		file_paths.push_back(path.string());
	} else if (fs::is_directory(path)) 
	{   // Step through each file, add only valid IFF files.
		for (auto& f : fs::directory_iterator(path)) {
			file_paths.push_back(f.path());
		}
	}
	return file_paths;
}


int main(int argc, char* argv[])
{
	Renderer ilbm_viewer;
	
	string path;
	auto generating_test_files = false;
	auto show_help = false;
	auto cli = lyra::cli_parser()
		     | lyra::help(show_help)
		     | lyra::opt(generating_test_files)["-g"]["--gentest"]("Generate testing data.")
		     | lyra::arg(path, "path")("File or folder to view.");

	auto result = cli.parse({ argc, argv });

	if (path.empty()) {
		cout << "You need to supply a file path.\n";
		return 1;
	}
	
	if (!CheckPath(path)) {
		return 1; 
	}

	const auto file_paths = GetPathsInFolder(fs::absolute(path));
	
	if (file_paths.size() == 0) {
		cout << "No file found in folder.\n";
		return 1;
	}

	// Add files to collection (=open them for viewing).
	if (!ilbm_viewer.AddImages(file_paths) && ilbm_viewer.Viewable()) {
		cout << "No suitable IFF files found in folder.\n";
		return 1;
	}

	// If we are outputting test files, we do this here, then exit.
	if (generating_test_files) 
	{   // Add paths to the generated files.
		const auto root = fs::absolute(path).parent_path().parent_path();

		vector<string> input_files;
		for (auto& f : file_paths) {
			input_files.emplace_back(f.string());
		}

		for (auto& f : file_paths) {
			fs::path n = root;
			n /= "ILBMviewer_test";
			n /= "test dumps";
			n /= (fs::absolute(f).stem().string() + ".tst");

			ofstream testfile(n, ios::binary);
			if (testfile.is_open()) {
				auto data = ilbm_viewer.GetData(ilbm_viewer.GetFileByAbspath(f));
				for (auto& d : data) {
					testfile.write((char*)&d, sizeof(d));
				}
				testfile.close();
			}
		}
		return 0;
	}

	// Supply args for image folder to view.
	if (ilbm_viewer.Construct(320, 240, 2, 2, false, true)) {
		ilbm_viewer.Start();
	}
	return 0;
}
