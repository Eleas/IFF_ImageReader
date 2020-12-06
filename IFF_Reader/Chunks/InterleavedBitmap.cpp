#include "InterleavedBitmap.h"
#include "Unknown.h"
#include <array>
#include <iostream>
#include <map>

using std::array;
using std::make_shared;
using std::map;
using std::min;
using std::shared_ptr;

// ILBM consists of multiple chunks, fabricated here.
// Detects chunk type, fabricates. Unknown chunks beyond the first are logged.
void IFFReader::ILBM::FabricateChunks(bytestream &stream) {
  while (stream.good()) {
    const string tag{read_tag(stream)};

    // This describes list of available chunks.
    const map<string, CHUNK_T> chunks = {{"BMHD", CHUNK_T::BMHD},
                                         {"CMAP", CHUNK_T::CMAP},
                                         {"CAMG", CHUNK_T::CAMG},
                                         {"BODY", CHUNK_T::BODY}};

    // Identify chunk.
    const auto found_chunk =
        chunks.find(tag) != chunks.end() ? chunks.at(tag) : CHUNK_T::UNKNOWN;

    if (body_ && !stream.good()) {
      return; // No more data can exist after BODY tag, so parsing terminates.
    }

    // Build objects or log the attempt.
    switch (found_chunk) {
    case CHUNK_T::BMHD:
      header_ = make_shared<BMHD>(BMHD(stream));
      break;
    case CHUNK_T::CMAP:
      cmap_ = make_shared<CMAP>(CMAP(stream));
      break;
    case CHUNK_T::CAMG:
      camg_ = make_shared<CAMG>(CAMG(stream));
      break;
    case CHUNK_T::BODY:
      body_ = make_shared<BODY>(BODY(stream));
      break;
    case CHUNK_T::UNKNOWN:
    default:
      unknown_chunks[tag] = make_shared<UNKNOWN>(UNKNOWN(stream));
    }
  }
}

// Cribbed and slightly modified from Hacker's Delight 2nd Edition
array<uint8_t, 8> transpose8(const array<uint8_t, 8> &A) {
  array<uint8_t, 8> result;
  unsigned long long x = 0;

  for (int i = 0; i <= 7;
       i++) { // Load bytes from the input array, pack into x.
    x = x << 8 | A.at(7 - i);
  }

  // Rotate the bits in three passes.
  x = x & 0xAA55AA55AA55AA55LL | (x & 0x00AA00AA00AA00AALL) << 7 |
      (x >> 7) & 0x00AA00AA00AA00AALL;

  x = x & 0xCCCC3333CCCC3333LL | (x & 0x0000CCCC0000CCCCLL) << 14 |
      (x >> 14) & 0x0000CCCC0000CCCCLL;

  x = x & 0xF0F0F0F00F0F0F0FLL | (x & 0x00000000F0F0F0F0LL) << 28 |
      (x >> 28) & 0x00000000F0F0F0F0LL;

  for (int i = 7; i >= 0; i--) { // Store result into output array B.
    result.at(i) = {static_cast<uint8_t>(x)};
    x = x >> 8;
  }

  return move(result);
}

// Translates eight planar pixels to eight chunky pixels.
const array<uint8_t, 8> PlanarToChunky8(const vector<uint8_t> &bits,
                                        const int byte_position,
                                        const int scan_line_bytelength,
                                        const int raster_line_bytelength,
                                        const int bitplanes) {
  const int startline{(byte_position / scan_line_bytelength) *
                      raster_line_bytelength};

  int bytepos{startline + ((byte_position) % scan_line_bytelength)};

  array<uint8_t, 8> bytes_to_use{0};

  for (int n = 0; n < bitplanes; ++n) {
    bytes_to_use.at(n) = bits.at(bytepos);
    bytepos += (scan_line_bytelength);
  }

  return move(transpose8(bytes_to_use));
}

// Note that screen data (points) differs from color values (clut).
const vector<uint8_t> IFFReader::ILBM::ComputeScreenData() const {
  // Pixel buffer set as one single allocation rather than many.
  const uint32_t pixel_count = width() * height();
  vector<uint8_t> data(pixel_count);

  unsigned int bit_position{0};
  array<uint8_t, 8> arr;

  const unsigned int scan_line_bytelength =
      (width() / 8) +
      (width() % 8 != 0 ? 1 : 0); // Round up scanline width to nearest byte.

  const unsigned int raster_line_bytelength{scan_line_bytelength *
                                            bitplanes_count()};

  while (bit_position < pixel_count) {
    const int limit = pixel_count - bit_position;
    const auto bytelimit = min(limit, 8);

    arr = PlanarToChunky8(extracted_bitplanes_, bit_position / 8,
                          scan_line_bytelength, raster_line_bytelength,
                          bitplanes_count());

    for (int i = 0; i < bytelimit; ++i) {
      data.at(bit_position++) = {arr.at(i)};
    }
  }

  return move(data);
}

IFFReader::ILBM::ILBM(bytestream &stream) {
  FabricateChunks(stream);
  // Determine if this should be EHB or HAM6 or HAM8

  // Add new data chunk with all data, not just OCS and CAMG.

  ComputeInterleavedBitplanes();

  if (camg_ && camg_->GetModes().ExtraHalfBrite) {
    color_lookup_ = make_shared<IFFReader::ColorLookupEHB>(
        cmap_->GetColorsEHB(screen_data_, bitplanes_count()));
  } else if (camg_ && camg_->GetModes().HoldAndModify) {
    color_lookup_ = make_shared<IFFReader::ColorLookupHAM>(
        cmap_->GetColorsHAM(screen_data_, bitplanes_count()));
  } else {
    color_lookup_ = make_shared<IFFReader::ColorLookup>(
        cmap_->GetColors(screen_data_, bitplanes_count()));
  }

  const auto mangled = color_lookup_->MightBeMangledOCS();
}

const uint32_t IFFReader::ILBM::width() const { return header_->GetWidth(); }

const uint32_t IFFReader::ILBM::height() const { return header_->GetHeight(); }

const uint16_t IFFReader::ILBM::bitplanes_count() const {
  return header_->GetBitplanesCount();
}

const uint32_t IFFReader::ILBM::color_at(const unsigned int x,
                                         const unsigned int y) const {
  return color_lookup_->at(static_cast<uint32_t>(
      (static_cast<uint64_t>(y) * static_cast<uint64_t>(width())) +
      static_cast<uint64_t>(x)));
}

const bool IFFReader::ILBM::allows_ocs_correction() const {
  return color_lookup_->MightBeMangledOCS();
}

const bool IFFReader::ILBM::using_ocs_correction() const {
  return color_lookup_->UsingOCSColorCorrection();
}

void IFFReader::ILBM::color_correction(const bool enable) {
  color_lookup_->AdjustForOCS(enable);
}

const bytefield IFFReader::ILBM::FetchData(const uint8_t compression) const {
  if (!body_) {
    return bytefield(); // empty
  }

  switch (compression) {
  case 1:
    return body_->GetUnpacked_ByteRun1();
  default:
    return body_->GetRawData();
  }
}

void IFFReader::ILBM::ComputeInterleavedBitplanes() {
  if (extracted_bitplanes_.empty()) {
    extracted_bitplanes_ = FetchData(header_->CompressionMethod());
  }

  screen_data_ = ComputeScreenData();
}
