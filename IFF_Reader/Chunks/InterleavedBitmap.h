#pragma once
#include "BitmapHeader.h"
#include "Body.h"
#include "Chunk.h"
#include "ColorMap.h"
#include "CommodoreAmiga.h"
#include "utility.h"

#include <map>

using std::map;
using std::shared_ptr;

namespace IFFReader {

// List of recognized chunk types.
enum class CHUNK_T { BMHD, CMAP, CAMG, BODY, UNKNOWN };

class ILBM : public CHUNK {
private:
  // Chunk map
  map<string, shared_ptr<CHUNK>> unknown_chunks;

  // Extracted image data
  bytefield extracted_bitplanes_;

  // replacement for pixels vector
  vector<uint8_t> screen_data_;
  shared_ptr<ColorLookup> color_lookup_;

  // Chunk data
  shared_ptr<BMHD> header_;
  shared_ptr<CMAP> cmap_;
  shared_ptr<CAMG> camg_;
  shared_ptr<BODY> body_;

  // Constructs supported ILBM chunks from stream.
  void FabricateChunks(bytestream &stream);

  // All valid ILBM files have a CMAP chunk for color data. ILBM format stores
  // a full byte per component in rgb, but OCS cannot display 8 bit color.
  // Well-formed OCS images instead repeat the high nibble for every component
  // for the purposes of IFF ILBM files. For example, $0055aa.
  //
  // A few early IFF readers generated malformed CMAP chunks, recognized by
  // setting the high nibble to 0; thus $fff becomes $0f0f0f. This is 
  // handled in the ColorLookup object.

  // Loads data, computes screen values.
  void ComputeInterleavedBitplanes();

  // Loads data from BODY tag, decompressing as appropriate.
  const bytefield FetchData(const uint8_t compression_method) const;

  // Translates bitplanes into chunky indices
  const vector<uint8_t> ComputeScreenData() const;

  // Fabricates correct palette lookup table.
  shared_ptr<IFFReader::ColorLookup> ColorLookupFactory();

public:
  ILBM(bytestream &stream);

  // ILBM graphics functions. Replace with Displayable API, allowing
  // all image formats to display in the same way.

  // Whether this is OCS, AGA, or something else.
  const IFFReader::Chipset InferChipset() const;

  // Whether this is EHB, HAM6, or something else.
  const IFFReader::ScreenMode InferScreenMode() const;

  // Determine how many colors the image has.
  const size_t DefinedColorsCount() const;

  // Width in pixels.
  const uint32_t width() const;

  // Height in pixels.
  const uint32_t height() const;

  // Number of bitplanes, not including mask.
  const uint16_t bitplanes_count() const;

  // Access pixels.
  const uint32_t color_at(const unsigned int x, const unsigned int y) const;

  // Whether OCS color correction is relevant.
  const bool allows_ocs_correction() const;

  // Whether OCS color correction is currently applied.
  const bool using_ocs_correction() const;

  // Enable or disable OCS color correction.
  void color_correction(const bool enable);

  // Returns basic information about the image.
  const string GetImageInfo() const;

  // Gets the number of colors currently displayed.
  const size_t ColorCount() const;
};
} // namespace IFFReader
