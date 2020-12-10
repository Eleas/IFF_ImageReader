#pragma once
#include "Chunk.h"
#include "ColorLookup.h"
#include "utility.h"

namespace IFFReader {

class CMAP : public CHUNK {
  vector<uint32_t> palette_;
  bool lower_nibbles_zero;
  bool nibbles_mirrored;

  // Gets whether lower nibbles echo upper nibbles
  // for every color. Excludes anti-alias byte.
  // (0xff113344  == true,
  //  0xff103548  == false)
  const bool LowerNibblesDuplicated() const;

  // Gets whether lower nibbles are all zero
  // for every color. Excludes anti-alias byte.
  // (0xff113344  == false,
  //  0xff103040  == true)
  const bool LowerNibblesZero() const;

public:
  CMAP();
  CMAP(bytestream &stream);

  // Gets chipset characteristics of the palette.
  const Chipset InferredChipset() const;

  // Counts the number of uniquely specified colors.
  const size_t DefinedColorsCount() const;

  // Corrects missing color information (use for potential OCS IFF images only)
  void CorrectOCSBrightness();

  // Extracts palette from raw data.
  const ColorLookup GetColors(vector<uint8_t> &data,
                              const uint16_t width_of_scanline,
                              const uint16_t bitplanes,
                              const BasicChipset chipset) const;

  // Extracts palette from raw data using Extra Halfbrite.
  const ColorLookupEHB GetColorsEHB(vector<uint8_t> &data,
                                    const uint16_t width_of_scanline,
                                    const uint16_t bitplanes,
                                    const BasicChipset chipset) const;

  // Extracts palette from raw data using Hold-and-Modify.
  const ColorLookupHAM GetColorsHAM(vector<uint8_t> &data,
                                    const uint16_t width_of_scanline,
                                    const uint16_t bitplanes,
                                    const BasicChipset chipset) const;
};
} // namespace IFFReader
