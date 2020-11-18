#include "ColorMap.h"
#include "Chunk.h"
#include "utility.h"

IFFReader::CMAP::CMAP() {}

// Extracts palette as a collection of colors.
IFFReader::CMAP::CMAP(bytestream &stream) : CHUNK(stream) {
  const auto color_count{GetSize() / 3}; // 3 bytes: R,G,B.

  // We store our colordata on the format 0xff gg bb rr.
  for (unsigned int i = 0; i < color_count; ++i) {
    palette_.push_back(read_byte(stream) | (read_byte(stream) << 8) |
                       (read_byte(stream) << 16) | (0xff << 24));
  }
}

void IFFReader::CMAP::CorrectOCSBrightness() {
  if (palette_.size() > 32) {
    return;
  }

  // Test that each low nibble is 0 for each color.
  for (auto &c : palette_) {
    if ((c & 0x000f0f0f) != 0) {
      return;
    }
  }

  for (auto &p :
       palette_) { // For each color, copy each high nibble into low nibble.
    p |= ((p & 0x00ffffff) >> 4);
  }
}

// Object that handles palette lookups.
const IFFReader::ColorLookup
IFFReader::CMAP::GetColors(vector<uint8_t> &data,
                           const uint16_t bitplanes) const {
  return ColorLookup(palette_, data, bitplanes);
}

// Object that handles palette lookups.
const IFFReader::ColorLookupEHB
IFFReader::CMAP::GetColorsEHB(vector<uint8_t> &data,
                              const uint16_t bitplanes) const {
  return ColorLookupEHB(palette_, data, bitplanes);
}

// Object that handles palette lookups.
const IFFReader::ColorLookupHAM
IFFReader::CMAP::GetColorsHAM(vector<uint8_t> &data,
                              const uint16_t bitplanes) const {
  return ColorLookupHAM(palette_, data, bitplanes);
}
