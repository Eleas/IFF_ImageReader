
// O------------------------------------------------------------------------------O
// | IFF Reader										                              |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include "lyra/lyra.hpp"
#include "FileData.h"
#include "RenderEngine.h"
#include <functional>
#include <memory>
#include <thread>


using std::cout;
using std::ios;
using std::ofstream;
using std::ref;
using std::thread;

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
{ // Only called in debug mode. Used to build files for regression test.
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
			const auto data = 
				ilbm_viewer.GetData(ilbm_viewer.GetFilePosByAbspath(f));
			for (auto& d : data) {
				testfile.write((char*)&d, sizeof(d));
			}
			testfile.close();
		}
	}
}


// Lets the loader/unpacker work side by side with renderer.
void add_images_threadholder(Renderer& ilbm_viewer, 
	const vector<fs::path>& file_paths) 
{
	const bool images_to_view = ilbm_viewer.AddImages(file_paths);

	if (images_to_view == false && ilbm_viewer.Viewable()) 
	{   // Notify viewer that no valid files exist, to let it terminate.
		ilbm_viewer.TerminateProcess(2);
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

	// We open a separate thread for unpacking the images. It is their job
	// to keep track of whether or not they're loaded.
	thread t1(add_images_threadholder, ref(ilbm_viewer), ref(file_paths));

	if (generating_test_files) {
#ifdef _DEBUG
		t1.join(); // Test file output needs no parallelism.

		// If we are outputting test files, we do this here, then exit.
		GenerateAndStoreTestFiles(file_paths, path, ilbm_viewer);
		return 0;
#endif
		cout << "Regression test file dump not supported in release version.\n";
		return 1;
	}

	t1.detach(); // Separate process.

	if (ilbm_viewer.Construct(320, 240, 2, 2, false, true)) {
		ilbm_viewer.Start();
	}

	if (ilbm_viewer.GetHaltCode() == 2) {
		cout << "No suitable IFF files found in folder.\n";
		return 2;
	}

	return 0;
}
