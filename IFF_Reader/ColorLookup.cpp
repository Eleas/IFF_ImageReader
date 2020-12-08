#include "ColorLookup.h"
#include <algorithm>

using std::all_of;
using std::for_each;

// Not const ref due to some kind of pointer magic, simply to convey that no
// mutable data is passed.
IFFReader::ColorLookup::ColorLookup(const vector<uint32_t> &colors,
                                    vector<uint8_t> &data,
                                    const uint16_t bitplanes)
    : colors_(colors), colors_scratch_(colors), data_(data),
      bitplane_count_(bitplanes), color_correction_(false) {
}

// Yields the base palette.
const vector<uint32_t> &IFFReader::ColorLookup::GetColors() const {
  return colors_scratch_;
}

// Yields the image binary contents (translated into chunky orientation).
const vector<uint8_t> &IFFReader::ColorLookup::GetData() const { return data_; }

// We need to use bitplanes in derived classes.
const int IFFReader::ColorLookup::BitplaneCount() const {
    return bitplane_count_;
}

// Looks up a color at the given pixel position.
const uint32_t IFFReader::ColorLookup::at(const size_t index) {
  return colors_scratch_.at(GetData().at(index));
}

// OCS images are sometimes stored incorrectly, with the low nibbles
// set to zero. If adjustment is requested, we simply mirror
// high nibbles to low nibbles. If not, we use the unmodified list.
void IFFReader::ColorLookup::AdjustForOCS(const bool adjust) {
  color_correction_ = adjust; // Store choice.
  colors_scratch_ = colors_;  // Restore original palette.

  if (adjust == false) {
    return;
  }

  // OCS color correction, quick and dirty.
  for_each(begin(colors_scratch_), end(colors_scratch_),
           [](uint32_t &c) { c |= ((c & 0x00f0f0f0) >> 4); });
}

const bool IFFReader::ColorLookup::UsingOCSColorCorrection() const {
  return color_correction_;
}

// Test if palette colors have same high nibbles as low nibbles.
const bool IFFReader::ColorLookup::LowerNibblesDuplicated() const {
  return all_of(begin(colors_), end(colors_), [](uint32_t c) {
    return (0x000f0f0f & c) == ((0x00f0f0f0 & c) >> 4);
  });
}

// Test if all palette colors have low nibbles that are zero.
const bool IFFReader::ColorLookup::LowerNibblesZero() const {
  return all_of(begin(GetColors()), end(GetColors()),
                [](uint32_t c) { return (0x000f0f0f & c) == 0; });
}

IFFReader::ColorLookupEHB::ColorLookupEHB(const vector<uint32_t> &colors,
                                          vector<uint8_t> &data,
                                          const uint16_t bitplanes)
    : ColorLookup(colors, data, bitplanes) {}

// Looks up a color at the given pixel position.
const uint32_t IFFReader::ColorLookupEHB::at(const size_t index) {
  const auto value = GetData().at(index);
  const auto &colors = GetColors();

  // For colors 32-63, halve each regular color value.
  return (value < colors.size())
             ? colors.at(value)
             : ((colors.at(value - 32) >> 1) | 0xFF000000) & 0xFF777777;
}

// Regular OCS images: if less than 5 bitplanes and lower nibbles
// of zero, colors are probably mangled.
const bool IFFReader::ColorLookup::MightBeMangledOCS() const {
  return UsingOCSColorCorrection() ||
         ((BitplaneCount() <= 6) && LowerNibblesZero());
}

// Test if this is definitely AGA, i.e. cannot possibly be OCS.
const bool IFFReader::ColorLookup::AgaColorDepth() const
{
    if (bitplane_count_ >= 6) {
        return false;
    }
    return !(LowerNibblesZero() || LowerNibblesDuplicated());
}

// Halfbrite images always use 6 bitplanes.
const bool IFFReader::ColorLookupEHB::MightBeMangledOCS() const {
  return UsingOCSColorCorrection() || LowerNibblesZero();
}

// HAM image: if HAM6 (six bitplanes) and lower nibbles
// of zero, colors are probably mangled.
const bool IFFReader::ColorLookupHAM::MightBeMangledOCS() const {
  return UsingOCSColorCorrection() ||
         ((BitplaneCount() == 6) && LowerNibblesZero());
}

IFFReader::ColorLookupHAM::ColorLookupHAM(const vector<uint32_t> &colors,
                                          vector<uint8_t> &data,
                                          const uint16_t bitplanes)
    : ColorLookup(colors, data, bitplanes), previous_color_(0) {}

// For a HAM image, the at method is different. It checks the
// two HAM bits, and gets regular color if it's 00. Otherwise, it looks
// at the previous colors, and alters red, green or blue by the four first
// bits.
const uint32_t IFFReader::ColorLookupHAM::at(const size_t index) {
  const auto given_value = GetData().at(index);
  const auto change = (given_value & 0xf);

  switch (given_value >> 4) {
  case 0:
    previous_color_ = GetColors().at(given_value);
    break;
  case 1: // Red
    previous_color_ = ((previous_color_ & 0xffffff00) | change);
    break;
  case 2: // Green
    previous_color_ = ((previous_color_ & 0xffff00ff) | (change << 8));
    break;
  case 3: // Blue
    previous_color_ = ((previous_color_ & 0xff00ffff) | (change << 16));
    break;
  }

  return previous_color_;
}
