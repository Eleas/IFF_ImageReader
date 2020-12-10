#include "utility.h"

namespace fs = std::filesystem;

// Reads the ASCII tag name (always four bytes).
const string IFFReader::read_tag(bytestream &stream) {
  char buffer[4];
  stream.read(reinterpret_cast<uint8_t *>(buffer), 4);

  return string(buffer, sizeof(buffer) / sizeof(char));
}

// The following three functions all do the same thing:
// each reads a big endian from a stream, converts it to
// little endian, and returns the result.

// Reads big endian longword (4 bytes), returns little endian.
const uint32_t IFFReader::read_long(bytestream &stream) {
  uint32_t buffer;
  stream.read(reinterpret_cast<uint8_t *>(&buffer), 4);

  uint32_t tmp = ((buffer << 8) & 0xFF00FF00) | ((buffer >> 8) & 0xFF00FF);
  buffer = (tmp << 16) | (tmp >> 16);

  return buffer;
}

// Reads big endian word (2 bytes), returns little endian.
const uint16_t IFFReader::read_word(bytestream &stream) {
  uint16_t buffer;
  stream.read(reinterpret_cast<uint8_t *>(&buffer), 2);

  return (buffer >> 8) | (buffer << 8);
}

// Reads byte.
const uint8_t IFFReader::read_byte(bytestream &stream) {
  uint8_t buffer;
  stream.read(reinterpret_cast<uint8_t *>(&buffer), 1);

  return buffer;
}

// Checks that file path exists.
const bool IFFReader::CheckPath(const string path) { // Patch for powershell bug
  auto temp_path = path;
  if (!temp_path.empty() && temp_path.back() == '"') {
    temp_path.pop_back();
  }
  const auto abspath = fs::absolute(temp_path);

  if (fs::exists(abspath.string())) {
    return true;
  }

  return false;
}

// Returns collection of all filepaths in folder.
const vector<fs::path> IFFReader::GetPathsInFolder(
    const fs::path &path) { // We now have the folder path.
  vector<fs::path> file_paths;

  // Get file candidates.
  if (fs::is_regular_file(path)) {
    file_paths.push_back(path.string());
    return file_paths;
  }

  if (fs::is_directory(
          path)) { // Step through each file, add only valid IFF files.
    for (auto &f : fs::directory_iterator(path)) {
      file_paths.push_back(f.path());
    }
  }
  return file_paths;
}
