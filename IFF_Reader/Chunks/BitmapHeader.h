#pragma once
#include "Chunk.h"
#include "utility.h"

namespace IFFReader {
// Instead of nulling them, set them in initializer.
class BMHD : public CHUNK {
  uint16_t width_;
  uint16_t height_;
  uint16_t xcoordinate_; // X position (used in DPaint, etc)
  uint16_t ycoordinate_; // Y position (same)
  uint8_t bitplanes_;    // # of bitplanes, sans mask
  uint8_t masking_; // Masking technique used. 0 = none, 1 = has mask, 2 = has
                    // transparent color, 3 = lasso
  uint8_t compression_;    // Compression type used. 0 = none, 1 = byteRun1
  uint16_t transparency_;  // Transparent background color
  uint8_t x_aspect_ratio_; // Horizontal pixel size
  uint8_t y_aspect_ratio_; // Vertical pixel size
  uint16_t page_width_;    // Horizontal resolution of display device
  uint16_t page_height_;   // Vertical resolution of display device

public:
  BMHD();
  BMHD(bytestream &stream);

  // Screen width, in pixels.
  const uint16_t GetWidth() const;

  // Screen height, in pixels.
  const uint16_t GetHeight() const;

  // Note that this is not meaningful for planar images.
  const uint16_t GetBitplanesCount() const;

  // Returns the compression used. Typical ILBM compression is
  // byterun1, but there's at least one one other that I've seen.
  const uint8_t CompressionMethod() const;

  // Is a mask in play, and which?
  const uint8_t MaskUsed() const;
};
} // namespace IFFReader
