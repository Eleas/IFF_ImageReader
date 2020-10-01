#include "InterleavedBitmap.h"
#include "Unknown.h"
#include <map>

using std::map;
using std::make_shared;


// ILBM consists of multiple chunks, fabricated here.
// Detects chunk type, fabricates. Unknown chunks beyond the first are logged.
void IFFReader::ILBM::FabricateChunks(bytestream& stream)
{
	while (stream.good()) {
		const auto tag{ read_tag(stream) };

		// This describes list of available chunks.
		const map <string, CHUNK_T> chunks = {
			{ "BMHD", CHUNK_T::BMHD },
			{ "CMAP", CHUNK_T::CMAP },
			{ "CAMG", CHUNK_T::CAMG },
			{ "DPI ", CHUNK_T::DPI },
			{ "BODY", CHUNK_T::BODY }
		};

		// Identify chunk.
		const auto found_chunk = chunks.find(tag) != chunks.end() ? chunks.at(tag) : CHUNK_T::UNKNOWN;

		// Build objects or log.
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

		// Special handling. No more data can exist after BODY tag, so terminate. 
		// That's a hack; better to ensure !stream.good() after parsing BODY, or 
		// to count the bytes.
		if (found_chunk == CHUNK_T::BODY) {
			return;
		}
	}
}


// Computes one planar pixel to one chunky pixel.
const inline uint8_t PlanarToChunky(const std::vector<uint8_t>& bits,
	const int absolute_position,
	const int width,
	const int bitplanes)
{
	const auto scan_line_bytelength = (width / 8) +
		(width % 8 != 0 ? 1 : 0);	// Round up the scan line width to nearest byte.

	const auto raster_line_bytelength{ (scan_line_bytelength * bitplanes) };
	const auto startline{ ((absolute_position / scan_line_bytelength / 8) * raster_line_bytelength) };
	const auto startbyte{ startline + ((absolute_position / 8) % scan_line_bytelength) };
	const auto bitpos{ 7 - (absolute_position % 8) };	// we count from highest to lowest.

	uint8_t buffer{ 0 }, byte{ 0 };
	unsigned int bytepos{ 0 };

	for (uint8_t n = 0; n < bitplanes; ++n) {
		bytepos = startbyte + (n * scan_line_bytelength);
		byte = bits.at(bytepos);
		buffer |= ((1 << bitpos) & byte) != 0 ? 1 << n : 0;
	}

	return buffer;
}

// To do: Split this into compute screen values and compute colors.
// Move it all into a Screen object.




const vector<uint8_t> IFFReader::ILBM::ComputeScreenData() const
{
	// Pixel buffer is set as single allocation rather than many.
	const auto pixel_count{ width() * height() };
	vector<uint8_t> data(pixel_count);

	unsigned int bit_position{ 0 };
	uint8_t col;

	// Create P2C version that only takes bit position for bpl 0.
	// it shouldn't care about returning a pixel at all.
	while (bit_position < pixel_count) {
		col = PlanarToChunky(
			extracted_bitplanes_,
			bit_position,
			width(),
			bitplanes_count());

		data.at(bit_position++) = { col };
	}
	return move(data);
}


// We fabricate a Screen object here. Screen object gets a reference to the raw data,
// and takes header, camg, body tags.
IFFReader::ILBM::ILBM(bytestream& stream)
{
	FabricateChunks(stream);
	ComputeInterleavedBitplanes();

	stored_palette_ = GetPalette();
}


const uint32_t IFFReader::ILBM::width() const
{
	return header_->GetWidth();
}


const uint32_t IFFReader::ILBM::height() const
{
	return header_->GetHeight();
}


const uint16_t IFFReader::ILBM::bitplanes_count() const
{
	return header_->GetBitplanesCount();
}


// Eventually derive this as a single uint32_t color value, not a "pixel."
const IFFReader::color IFFReader::ILBM::at(unsigned int x, unsigned int y)
{
	const auto position = screen_data_.at(
		(static_cast<uint32_t>(y) * static_cast<uint32_t>(width())) 
		+ static_cast<uint32_t>(x));
	return stored_palette_.at(position);
}


// Perhaps an update palette function would be preferable, 
// followed by a simple getter.
const IFFReader::colors IFFReader::ILBM::GetPalette() const
{
	if (!cmap_) {
		return colors(); // empty
	}

	return cmap_->GetPalette((camg_) ? camg_->GetModes() : OCSmodes{ 0 });
}


const bytefield IFFReader::ILBM::FetchData(const uint8_t compression) const
{
	if (!body_) {
		return bytefield(); // empty
	}

	switch (compression) {
	case 1:		return body_->GetUnpacked_ByteRun1();
	default:	return body_->GetRawData();
	}
}



void IFFReader::ILBM::ComputeInterleavedBitplanes()
{
	if (extracted_bitplanes_.empty()) {
		extracted_bitplanes_ = FetchData(header_->CompressionMethod());
	}
	screen_data_ = ComputeScreenData();
}

