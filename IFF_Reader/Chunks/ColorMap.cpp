#include "ColorMap.h"
#include "Chunk.h"
#include "utility.h"
#include <algorithm>

using std::all_of;

IFFReader::CMAP::CMAP() : lower_nibbles_zero(false), nibbles_mirrored(false) {}

// Extracts palette as a collection of colors.
IFFReader::CMAP::CMAP(bytestream &stream)
    : CHUNK(stream), lower_nibbles_zero(false), nibbles_mirrored(false) {
  const auto color_count{GetSize() / 3}; // 3 bytes: R,G,B.

  // We store our colordata on the format 0xff gg bb rr.
  for (unsigned int i = 0; i < color_count; ++i) {
    palette_.push_back(read_byte(stream) | (read_byte(stream) << 8) |
                       (read_byte(stream) << 16) | (0xff << 24));
  }

  // Finally, we note whether all nibbles are mirrored high/low or
  // whether all have zero values for their low nibbles.
}

// Try to infer whether the color map is AGA or ECS.
const IFFReader::Chipset IFFReader::CMAP::InferredChipset() const {
  if (palette_.size() <= 32 && (lower_nibbles_zero || nibbles_mirrored)) {
    return Chipset::OCS;
  }

  return Chipset::AGA;
}

// Test if palette colors have same high nibbles as low nibbles.
const bool IFFReader::CMAP::LowerNibblesDuplicated() const {
  return all_of(begin(palette_), end(palette_), [](uint32_t c) {
    return (0x000f0f0f & c) == ((0x00f0f0f0 & c) >> 4);
  });
}

// Test if all palette colors have low nibbles that are zero.
const bool IFFReader::CMAP::LowerNibblesZero() const {
  return all_of(begin(palette_), end(palette_),
                [](uint32_t c) { return (0x000f0f0f & c) == 0; });
}

// Counts the number of uniquely specified colors.
const size_t IFFReader::CMAP::DefinedColorsCount() const {
  return palette_.size();
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
IFFReader::CMAP::GetColors(vector<uint8_t> &data, const uint16_t bitplanes,
                           const BasicChipset chipset) const {
  return ColorLookup(palette_, data, bitplanes, chipset);
}

// Object that handles palette lookups.
const IFFReader::ColorLookupEHB
IFFReader::CMAP::GetColorsEHB(vector<uint8_t> &data, const uint16_t bitplanes,
                              const BasicChipset chipset) const {
  return ColorLookupEHB(palette_, data, bitplanes, chipset);
}

// Object that handles palette lookups.
const IFFReader::ColorLookupHAM
IFFReader::CMAP::GetColorsHAM(vector<uint8_t> &data, const uint16_t bitplanes,
                              const BasicChipset chipset,
                              const uint16_t width_of_scanline) const {
  return ColorLookupHAM(palette_, data, bitplanes, chipset, width_of_scanline);
}
