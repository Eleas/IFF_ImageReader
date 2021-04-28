#include "ColorLookup.h"
#include <algorithm>

using std::all_of;
using std::for_each;

IFFReader::ColorLookup::ColorLookup(const vector<uint32_t> &colors,
  const vector<uint8_t> &data,
  const uint32_t width,
  const uint16_t bitplanes,
  const BasicChipset chipset)
  : colors_(colors), colors_scratch_(colors), data_(data),
  scanline_width_(width), bitplane_count_(bitplanes),
  lower_nibble_zero_(false), color_correction_enabled_(false),
  chipset_(chipset)
{
  // Some OCS files are saved incorrectly, with only the high nibble set.
  // If that's detected (i.e. all low nibbles are 0), then we offer
  // the ability to correct for that.
  // Thus, we first of all store whether or not the lower nibbles are zero.
  lower_nibble_zero_ = all_of(begin(GetColors()), end(GetColors()),
    [](uint32_t c) { return (0x000f0f0f & c) == 0; });
}

// Yields the base palette.
const vector<uint32_t> &IFFReader::ColorLookup::GetColors() const
{
  return colors_scratch_;
}

// Yields the image binary contents (translated into chunky orientation).
const vector<uint8_t> &IFFReader::ColorLookup::GetData() const
{
  return data_;
}

// Some forms of color lookup need awareness of bitplane count.
const int IFFReader::ColorLookup::BitplaneCount() const
{
  return bitplane_count_;
}

// Looks up a color at the given pixel position.
const uint32_t IFFReader::ColorLookup::at(const uint32_t x, const uint32_t y) {
  const auto index = static_cast<uint32_t>(
    (static_cast<uint64_t>(y) * static_cast<uint64_t>(Width())) +
    static_cast<uint64_t>(x));

  return colors_scratch_.at(GetData().at(index));
}

// OCS images are sometimes stored incorrectly, with the low nibbles
// set to zero. If adjustment is requested, we simply mirror
// high nibbles to low nibbles. If not, we use the unmodified list.
void IFFReader::ColorLookup::AdjustForOCS(const bool adjust) {
  color_correction_enabled_ = adjust; // Store choice.
  colors_scratch_ = colors_;          // Restore original palette.

  if (adjust == false) {
    return;
  }

  // OCS color correction, quick and dirty.
  for_each(begin(colors_scratch_), end(colors_scratch_),
    [](uint32_t &c) { c |= ((c & 0x00f0f0f0) >> 4); });
}

// Test if we're currently doing color correction for OCS images.
const bool IFFReader::ColorLookup::UsingOCSColorCorrection() const {
  return color_correction_enabled_;
}

// Test if it makes sense to offer correction for OCS color mangling.
const bool IFFReader::ColorLookup::IsOCSCorrectible() const {
  return lower_nibble_zero_ && chipset_ == BasicChipset::OCS;
}

const uint32_t IFFReader::ColorLookup::Width() const { return scanline_width_; }

const IFFReader::BasicChipset IFFReader::ColorLookup::Chipset() const {
  return chipset_;
}

IFFReader::ColorLookupEHB::ColorLookupEHB(const vector<uint32_t> &colors,
  const vector<uint8_t> &data,
  const uint32_t width,
  const uint16_t bitplanes,
  const BasicChipset chipset)
  : ColorLookup(colors, data, width, bitplanes, chipset) {}

// Looks up a color at the given pixel position.
const uint32_t IFFReader::ColorLookupEHB::at(const uint32_t x,
  const uint32_t y) {
  const auto value = GetData().at(Width());
  const auto &colors = GetColors();

  // For colors 32-63, halve each regular color value.
  return (value < colors.size())
    ? colors.at(value)
    : ((colors.at(value - 32) >> 1) | 0xFF000000) & 0xFF777777;
}

IFFReader::ColorLookupHAM::ColorLookupHAM(const vector<uint32_t> &colors,
  const vector<uint8_t> &data,
  const uint16_t width_of_scanline,
  const uint16_t bitplanes,
  const BasicChipset chipset)
  : ColorLookup(colors, data, width_of_scanline, bitplanes, chipset),
  previous_color_(0) {}

// For a HAM image, the at method is different. It checks the
// two HAM bits, and gets regular color if it's 00. Otherwise, it looks
// at the previous colors, and alters red, green or blue by the four
// (for HAM6) or six (for HAM8) first bits.
const uint32_t IFFReader::ColorLookupHAM::at(const uint32_t x,
  const uint32_t y)
{
  const auto index = static_cast<uint32_t>(
    (static_cast<uint64_t>(y) * static_cast<uint64_t>(Width())) +
    static_cast<uint64_t>(x));

  const auto given_value = GetData().at(index);

  const auto is_aga = Chipset() == BasicChipset::AGA;

  // What do these flags mean?
  enum class HAMmode { AsRegularColor = 0, ModifyBlue = 1, ModifyRed = 2, ModifyGreen = 3 };

  const auto HAM_flag = static_cast<HAMmode>(given_value >> (is_aga ? 6 : 4));
  const auto modify_part = is_aga ? (given_value & 0x3f) : (given_value & 0xf);

  // If the first pixel of a scan line, assume previous colour
  // is equal to palette index 0.
  if (HAM_flag != HAMmode::AsRegularColor && x == 0) {
    previous_color_ = GetColors().at(0);
  }

  // This is the new color value for the modified bit. This works slightly
  // different between HAM6 and HAM8.
  //
  // If we're in OCS mode, take the color-change nibble (say 0x04),
  // then repeat it in the high nibble (0x44).
  //
  // For AGA, we take the first 6 bits (say 0b00011010),
  // kick them up two bits (0b011010 00), then repeat the
  // highest two bits, resulting in (0b01101001).
  const auto modify_value = is_aga ? (modify_part << 2) | (modify_part >> 4)
    : modify_part | (modify_part << 4);

  switch (HAM_flag) {
  case HAMmode::AsRegularColor:
    previous_color_ = GetColors().at(given_value);
    break;
  case HAMmode::ModifyBlue: // modify blue (hold red and green)
    previous_color_ = ((previous_color_ & 0xff00ffff) | (modify_value << 16));
    break;
  case HAMmode::ModifyRed: // modify red (hold green and blue)
    previous_color_ = ((previous_color_ & 0xffffff00) | modify_value);
    break;
  case HAMmode::ModifyGreen: // modify green (hold red and blue)
    previous_color_ = ((previous_color_ & 0xffff00ff) | (modify_value << 8));
    break;
  }

  return previous_color_;
}
