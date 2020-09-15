#pragma once
#include <fstream>

using std::basic_ifstream;
using std::string;

namespace IFFImageReader {
	typedef basic_ifstream<uint8_t> bytestream;

	// Helper functions. Reads stream and handles endianness.
	const string read_tag(bytestream& stream) {
		char temptag[4];
		stream.read(reinterpret_cast<uint8_t*>(temptag), 4);
		return string(temptag, sizeof(temptag) / sizeof(char));
	}

	const uint32_t read_long(bytestream& stream) {
		uint32_t buffer;
		stream.read(reinterpret_cast<uint8_t*>(&buffer), 4);
		uint32_t tmp = ((buffer << 8) & 0xFF00FF00) | ((buffer >> 8) & 0xFF00FF); 
		buffer = (tmp << 16) | (tmp >> 16);
		return buffer;
	}

	const uint16_t read_word(bytestream& stream) {
		uint16_t buffer;
		stream.read(reinterpret_cast<uint8_t*>(&buffer), 2);
		return (buffer >> 8) | (buffer << 8);
	}

	const uint8_t read_byte(bytestream& stream) {
		uint8_t buffer;
		stream.read(reinterpret_cast<uint8_t*>(&buffer), 1);
		return buffer;
	}
}