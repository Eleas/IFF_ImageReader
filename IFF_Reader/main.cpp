
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


// Handler class. Needs a better name.
class ImageFile 
{
	fs::path filepath;
	unique_ptr<IFFReader::File> file;
	shared_ptr<IFFReader::ILBM> ilbm;


public:
	const string ErrorMessage(const IFFReader::File& f) const
	{
		using IFFReader::IFF_ERRCODE;

		switch (file->GetError()) {
		case IFF_ERRCODE::FILE_NOT_FOUND:
			return "No valid IFF file found. Did you check the file path?\n";
		case IFF_ERRCODE::COULD_NOT_PARSE_AS_IFF:
			return "IFF file mangled; cannot open.\n";
		case IFF_ERRCODE::COULD_NOT_PARSE_HEAD:
			return "IFF file mangled; failed to parse head.\n";
		case IFF_ERRCODE::COULD_NOT_PARSE_BODY:
			return "IFF file mangled; failed to parse body.\n";
		default:
			if (file->GetType() == IFFReader::IFF_T::UNKNOWN_FORMAT) {
				return "This ILBM has an as-yet unsupported format.\n";
			}
		}
		return string();
	}


public:
	ImageFile(const fs::path& path) : filepath(path) 
	{
		file = unique_ptr<IFFReader::File>
			(new IFFReader::File(path.string()));

		if (const string error_text = ErrorMessage(*file.get()); 
			!error_text.empty()) 
		{
			cout << error_text;
			return;
		}

		ilbm = make_shared<IFFReader::ILBM>(*file->AsILBM());
	}


public:
	ImageFile() {}


public:
	shared_ptr<IFFReader::ILBM> Get() const 
	{ 
		return ilbm; 
	}


public:
	const bool IsPath(const fs::path path) const 
	{ 
		return path == filepath;
	};


public:
	const bool OffersOCSColourCorrection() const;


public:
	const bool UsingOCSColourCorrection() const;


public:
	const void ApplyOCSColourCorrection(const bool apply);
};


// Renderer class has started to become God object. Should 
// be subordinate to actual viewer via composition.
class Renderer : public olc::PixelGameEngine
{
	vector<ImageFile> images_;
	size_t current_image = 0;


private:
	void AddImage(ImageFile& img) 
	{
		images_.emplace_back(move(img));
	}


public:
	Renderer()
	{
		sAppName = "IFF reader";
	}
	double cyclic = 0;


public:
	const vector<uint32_t> GetData(const size_t n) const 
	{
		vector<uint32_t> contents;
		const auto data = images_.at(n).Get();
		for (unsigned int y = 0; y < data->height(); ++y) {
			for (unsigned int x = 0; x < data->width(); ++x) {
				const auto px = data->color_at(x, y);
				contents.push_back(px);
			}
		}
		return contents;
	}


public:
	const size_t GetFilePosByAbspath(const fs::path path) const 
	{
		for (size_t i = 0; i < images_.size(); ++i) {
			if (images_.at(i).IsPath(path)) {
				return i;
			}
		}
		return images_.size();
	}


public:
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


public:
	// Called once at the start, so create things here
	bool OnUserCreate() override 
	{
		DisplayImage();
		return true;
	}


private:
	const bool BackKeyReleased() 
	{
		return GetKey(olc::Key::LEFT).bReleased || GetKey(olc::Key::BACK).bReleased;
	}


private:
	const bool ForwardKeyReleased() 
	{
		return GetKey(olc::Key::RIGHT).bReleased || GetKey(olc::Key::SPACE).bReleased;
	}


private:
	bool OnUserUpdate(float fElapsedTime) override
	{
		const auto image_count = images_.size();

		// You can apply colour correction now.
		auto& this_image = images_.at(current_image);
		if (GetKey(olc::Key::O).bReleased && this_image.OffersOCSColourCorrection()) {
			const auto currently_enabled = this_image.UsingOCSColourCorrection();
			this_image.ApplyOCSColourCorrection(!currently_enabled);
			DisplayImage();
			return true;
		}

		if (images_.size() > 1) {
			const auto stored_image = current_image;
			if (BackKeyReleased()) {
				current_image = (current_image == 0) ? 
					image_count - 1 : 
					current_image - 1;
			}
			if (ForwardKeyReleased()) {
				current_image = (current_image == image_count - 1) ?
					0 : 
					current_image + 1;
			}
			if (stored_image != current_image) {
				Clear(olc::BLACK);
				DisplayImage();
			}
		}

		const auto exit_key_pressed = 
			GetKey(olc::Key::ESCAPE).bPressed ||
			GetKey(olc::Key::ENTER).bPressed || 
			GetKey(olc::Key::Q).bPressed ||
			GetKey(olc::Key::X).bPressed;

		return (!exit_key_pressed);	// Close viewer on keypress.
	}


public:
	const bool Viewable() const 
	{
		return images_.size() != 0;
	}


public:
	const bool AddImages(const vector<fs::path>& paths) 
	{   // Add files to collection (=open them for viewing).
		bool file_added = false;
		for (auto& f : paths) {
			if (auto image = ImageFile(f.string()) ; image.Get()) {
				AddImage(image);
				file_added = true;
			}
		}
		return file_added;
	}
};


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
		return file_paths;
	} 
	
	if (fs::is_directory(path)) 
	{   // Step through each file, add only valid IFF files.
		for (auto& f : fs::directory_iterator(path)) {
			file_paths.push_back(f.path());
		}
	}
	return file_paths;
}


void GenerateAndStoreTestFiles(const vector<fs::path>& file_paths, 
	const fs::path& path, 
	const Renderer& ilbm_viewer) 
{ // Should only be available in debug mode. Used to build files for regression test.
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
			const auto data = ilbm_viewer.GetData(ilbm_viewer.GetFilePosByAbspath(f));
			for (auto& d : data) {
				testfile.write((char*)&d, sizeof(d));
			}
			testfile.close();
		}
	}
}


int main(int argc, char* argv[])
{
	Renderer ilbm_viewer;
	
	string path;
	auto generating_test_files = false;
	auto show_help = false;
	const auto cli = lyra::cli_parser()
		     | lyra::help(show_help)
		     | lyra::opt(generating_test_files)["-g"]["--gentest"]("Generate testing data.")
		     | lyra::arg(path, "path")("File or folder to view.");

	const auto result = cli.parse({ argc, argv });

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

	if (generating_test_files) 
	{   // If we are outputting test files, we do this here, then exit.
#ifdef _DEBUG
		GenerateAndStoreTestFiles(file_paths, path, ilbm_viewer);
		return 0;
#endif
		cout << "Regression test file dump not supported in release version.\n";
		return 1;
	}

	// Supply args for image folder to view.
	if (ilbm_viewer.Construct(320, 240, 2, 2, false, true)) {
		ilbm_viewer.Start();
	}
	return 0;
}


const bool ImageFile::OffersOCSColourCorrection() const
{
	return ilbm->allows_ocs_correction();
}


const bool ImageFile::UsingOCSColourCorrection() const {
	return ilbm->using_ocs_correction();
}


const void ImageFile::ApplyOCSColourCorrection(const bool apply) 
{
	ilbm->color_correction(apply);
}
