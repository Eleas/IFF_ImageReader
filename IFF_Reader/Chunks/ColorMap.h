#pragma once
#include "Chunk.h"
#include "ColorLookup.h"
#include "utility.h"

namespace IFFReader {

class CMAP : public CHUNK {
  vector<uint32_t> palette_;
  bool full_OCS_compatibility;
  bool potential_mangled_OCS;

public:
  CMAP();
  CMAP(bytestream &stream);

  // Corrects missing color information (use for potential OCS IFF images only)
  void CorrectOCSBrightness();

  // Extracts palette from raw data.
  const ColorLookup GetColors(vector<uint8_t> &data,
                              const uint16_t bitplanes) const;

  // Extracts palette from raw data using Extra Halfbrite.
  const ColorLookupEHB GetColorsEHB(vector<uint8_t> &data,
                                    const uint16_t bitplanes) const;

  // Extracts palette from raw data using Hold-and-Modify.
  const ColorLookupHAM GetColorsHAM(vector<uint8_t> &data,
                                    const uint16_t bitplanes) const;
};
} // namespace IFFReader
