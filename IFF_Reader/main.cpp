
// O------------------------------------------------------------------------------O
// | IFF Reader |
// O------------------------------------------------------------------------------O

#define OLC_PGE_APPLICATION
#include "FileData.h"
#include "RenderEngine.h"
#include "lyra/lyra.hpp"
#include <memory>
#include <thread>

using std::cout;
using std::ios;
using std::ofstream;
using std::ref;
using std::thread;

void GenerateAndStoreTestFiles(
    const vector<fs::path> &file_paths, const fs::path &path,
    const Renderer &ilbm_viewer) { // Only called in debug mode. Used to build
                                   // files for regression test.
  const auto root = fs::absolute(path).parent_path().parent_path();

  vector<string> input_files;
  for (auto &f : file_paths) {
    input_files.emplace_back(f.string());
  }

  for (auto &f : file_paths) {
    fs::path n = root;
    n /= "ILBMviewer_test";
    n /= "test dumps";
    n /= (fs::absolute(f).stem().string() + ".tst");

    ofstream testfile(n, ios::binary);
    if (testfile.is_open()) {
      const auto data = ilbm_viewer.GetData(ilbm_viewer.GetFilePosByAbspath(f));
      for (auto &d : data) {
        testfile.write((char *)&d, sizeof(d));
      }
      testfile.close();
    }
  }
}

// Lets the loader/unpacker work side by side with renderer.
void add_images_threadholder(Renderer &ilbm_viewer,
                             const vector<fs::path> &file_paths) {
  const bool images_to_view = ilbm_viewer.AddImages(file_paths);

  if (images_to_view == false &&
      ilbm_viewer.Viewable()) { // Notify viewer that no valid files exist, to
                                // let it terminate.
    ilbm_viewer.BreakNoValidIFF();
  }
}

int main(int argc, char *argv[]) {
  Renderer ilbm_viewer;

  string path;
  auto generating_test_files = false;
  auto show_help = false;
  const auto cli = lyra::cli_parser() | lyra::help(show_help) |
                   lyra::opt(generating_test_files)["-g"]["--gentest"](
                       "Generate testing data.") |
                   lyra::arg(path, "path")("File or folder to view.");

  const auto result = cli.parse({argc, argv});

  if (path.empty()) {
    cout << "You need to supply a file path.\n";
    return 1;
  }

  if (!IFFReader::CheckPath(path)) {
    cout << "File or path " << fs::absolute(path).string() << " not found.\n";
    return 1;
  }

  const auto file_paths = IFFReader::GetPathsInFolder(fs::absolute(path));

  if (file_paths.size() == 0) {
    cout << "No file found in folder.\n";
    return 1;
  }

  // We open a separate thread for unpacking the images. It is their job
  // to keep track of whether or not they're loaded.
  thread image_parse_thread(add_images_threadholder, ref(ilbm_viewer),
                            ref(file_paths));

  if (generating_test_files) {
#ifdef _DEBUG
    image_parse_thread.join();

    // If we are outputting test files, we do this here, then exit.
    GenerateAndStoreTestFiles(file_paths, path, ilbm_viewer);

    return 0;
#endif
    cout << "Regression test file dump not supported in release version.\n";
    return 1;
  }

  if (ilbm_viewer.Construct(320, 240, 2, 2, false, true)) {
    ilbm_viewer.Start();
  }

  image_parse_thread.join(); // Thread rejoins main data.

  if (ilbm_viewer.InvalidIFF()) {
    cout << "No suitable IFF files found in folder.\n";
    return 2;
  }

  return 0;
}
