#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

using std::basic_ifstream;
using std::string;
using std::vector;

typedef basic_ifstream<uint8_t> bytestream;
typedef vector<uint8_t> bytefield;

namespace IFFReader {
// Reads the ASCII tag name (always four bytes).
const string read_tag(bytestream &stream);

// Reads big endian longword (4 bytes), returns little endian.
const uint32_t read_long(bytestream &stream);

// Reads big endian word (2 bytes), returns little endian.
const uint16_t read_word(bytestream &stream);

// Reads byte.
const uint8_t read_byte(bytestream &stream);

// Checks that file path exists.
const bool CheckPath(const string path);

// Returns collection of all filepaths in folder.
const vector<std::filesystem::path> GetPathsInFolder(
	const std::filesystem::path& path);
} // namespace IFFReader
