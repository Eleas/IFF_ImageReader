#include "FileData.h"


inline ILBMReader::CHUNK::CHUNK() : size_(0) 
{ 
}


inline ILBMReader::CHUNK::CHUNK(bytestream& stream) : size_(read_long(stream)) 
{
}


inline ILBMReader::CHUNK::~CHUNK() 
{
}


inline const uint32_t ILBMReader::CHUNK::GetSize() const 
{ 
	return size_; 
}


// Stores the tag as as a string. Only implemented for UNKNOWN.
void ILBMReader::CHUNK::AddTagLiteral(const string tag)
{
	// ... an identified chunk has a well known name, so 
	//	we do nothing here (see UNKNOWN chunk).
}


inline const string ILBMReader::read_tag(bytestream& stream)
{
	char temptag[4];
	stream.read(reinterpret_cast<uint8_t*>(temptag), 4);
	return string(temptag, sizeof(temptag) / sizeof(char));
}


// Reads big endian longword (4 bytes) and returns a little endian.
const uint32_t ILBMReader::read_long(bytestream& stream)
{
	uint32_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 4);
	uint32_t tmp = ((buffer << 8) & 0xFF00FF00) | ((buffer >> 8) & 0xFF00FF);
	buffer = (tmp << 16) | (tmp >> 16);
	return buffer;
}


// Reads big endian word (2 bytes) and returns a little endian.
inline const uint16_t ILBMReader::read_word(bytestream& stream) 
{
	uint16_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 2);
	return (buffer >> 8) | (buffer << 8);
}


// Reads byte.
inline const uint8_t ILBMReader::read_byte(bytestream& stream) 
{
	uint8_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 1);
	return buffer;
}


inline ILBMReader::BMHD::BMHD() : width_{ 0 }, height_{ 0 }, xcoordinate_{ 0 }, ycoordinate_{ 0 }, 
	bitplanes_{ 0 }, masking_{ 0 }, compression_{ 0 }, transparency_{ 0 }, x_aspect_ratio_{ 0 }, 
	y_aspect_ratio_{ 0 }, page_width_{ 0 }, page_height_{ 0 }
{ 
}


inline ILBMReader::BMHD::BMHD(bytestream& stream) : CHUNK(stream) 
{
	width_ = read_word(stream);
	height_ = read_word(stream);
	xcoordinate_ = read_word(stream);
	ycoordinate_ = read_word(stream);

	bitplanes_ = read_byte(stream);
	masking_ = read_byte(stream);
	compression_ = read_byte(stream);
	stream.ignore(1); // 1 byte padding
	transparency_ = read_word(stream);

	x_aspect_ratio_ = read_byte(stream);
	y_aspect_ratio_ = read_byte(stream);

	page_width_ = read_word(stream);
	page_height_ = read_word(stream);
}


inline const uint16_t ILBMReader::BMHD::GetWidth() const 
{ 
	return width_; 
}


inline const uint16_t ILBMReader::BMHD::GetHeight() const 
{
	return height_; 
}


inline const uint16_t ILBMReader::BMHD::GetBitplanesCount() const 
{ 
	return bitplanes_; 
}


const uint8_t ILBMReader::BMHD::Compression() const
{
	return compression_;
}


inline ILBMReader::CMAP::CMAP() 
{
}


// Extracts palette as a collection of colors.
inline ILBMReader::CMAP::CMAP(bytestream& stream) : CHUNK(stream) 
{
	palette_.clear();
	const auto color_count = GetSize() / 3;
	palette_.resize(color_count);

	for (auto& color : palette_) {
		color = { 
			read_byte(stream),
			read_byte(stream),
			read_byte(stream) 
		};
	}
}


const vector<ILBMReader::color> ILBMReader::CMAP::GetPalette() const 
{ 
	return palette_; 
}


inline ILBMReader::CAMG::CAMG() 
{
}


inline ILBMReader::CAMG::CAMG(bytestream& stream) : CHUNK(stream) 
{ 
	contents_ = read_long(stream); 
}


inline ILBMReader::DPI::DPI() 
{	
}


inline ILBMReader::DPI::DPI(bytestream& stream) : CHUNK(stream) 
{ 
	contents_ = read_long(stream); 
}


inline ILBMReader::BODY::BODY()
{
}


// BODY data fetched from stream as raw bytes.
inline ILBMReader::BODY::BODY(bytestream& stream) : CHUNK(stream) 
{
	for (uint32_t i = 0; i < GetSize(); ++i) {
		raw_data_.emplace_back(read_byte(stream));
	}
}


const vector<uint8_t>& ILBMReader::BODY::GetRawData() const
{
	return raw_data_;
}


// Unpacks raw data using ByteRun1 encoding. 
//	[http://amigadev.elowar.com/read/ADCD_2.1/Devices_Manual_guide/node01C0.html]
const vector<uint8_t> ILBMReader::BODY::GetUnpacked_ByteRun1() const
{
	const auto original_size = raw_data_.size();
	size_t position = 0;

	vector<uint8_t> unpacked_data; // Destination.

	while (position < original_size) {
		const auto value = static_cast<int8_t>(raw_data_.at(position++)); // Make it signed.

		/*
		[Some versions of Adobe Photoshop incorrectly use the n = 128 no - op as a repeat code, which breaks
		 strictly conforming readers. To read Photoshop ILBMs, allow the use of n = 128 as a repeat.This
		 is pretty safe, since no known program writes real no - ops into their ILBMs.The reason n = 128 is
		 a no - op is historical : the Mac Packbits buffer was only 128 bytes, and a repeat code of 128
		 generates 129 bytes.] 
		 
			if (value == -128) continue; 

		 */

		if (value >= 0) {
			for (int i = 0; i < value + 1; ++i) {
				unpacked_data.emplace_back(raw_data_.at(position++));
			}
		} else {
			for (int i = 0; i < (-value) + 1; ++i) {
				unpacked_data.emplace_back(raw_data_.at(position));
			}
			++position;
		}
	}

	return move(unpacked_data);
}


// Represents a tag not recognized.
ILBMReader::UNKNOWN::UNKNOWN()
{
}


// Unknown tags are bypassed without extracting data.
inline ILBMReader::UNKNOWN::UNKNOWN(bytestream& stream) : CHUNK(stream)
{
	stream.ignore(GetSize()); // Sod off, on to next chunk with ye.
}


// Logs the tag literal of unknown chunks.
void ILBMReader::UNKNOWN::AddTagLiteral(const string name)
{
	unknown_chunk_names_.push_back(name);
}


// Detects chunk type, fabricates. Unknown chunks beyond the first are logged.
void ILBMReader::ILBM::ChunkFactory(bytestream& stream) 
{
	const auto size = GetSize();
	const auto original_pos = stream.tellg();

	while (stream.good()) {
		const auto tag = read_tag(stream);
		const auto found_chunk = supported_chunks_.find(tag);
		const auto found_tag = (found_chunk != supported_chunks_.end()) ? found_chunk->second : CHUNK_T::UNKNOWN;

		// If the chunk was unknown and we already have a fabricated unknown chunk, don't make a new one.
		if (found_tag != CHUNK_T::UNKNOWN || chunks_.find(CHUNK_T::UNKNOWN) == chunks_.end()) {
			chunks_[found_tag] = ChunkFactoryInternals(stream, found_tag);
		}

		// Log the tag literal.
		chunks_[found_tag]->AddTagLiteral(tag);

		// No more data can exist after BODY tag, so terminate. This is a hack; better to ensure !stream.good() after
		// parsing BODY, or to count the bytes.
		if (found_tag == CHUNK_T::BODY) { 
			return; 
		}
	}
}


// Fabricates appropriate chunk from stream.
unique_ptr<ILBMReader::CHUNK> ILBMReader::ILBM::ChunkFactoryInternals(bytestream& stream, const CHUNK_T found_chunk) const
{
	unique_ptr<CHUNK> result;

	switch (found_chunk) {
		case CHUNK_T::BMHD:		return move(make_unique<BMHD>(BMHD(stream)));
		case CHUNK_T::CMAP:		return move(make_unique<CMAP>(CMAP(stream)));
		case CHUNK_T::CAMG:		return move(make_unique<CAMG>(CAMG(stream)));
		case CHUNK_T::DPI:		return move(make_unique<DPI>(DPI(stream)));
		case CHUNK_T::BODY:		return move(make_unique<BODY>(BODY(stream)));
		case CHUNK_T::UNKNOWN:	return move(make_unique<UNKNOWN>(UNKNOWN(stream)));
	}

	return result; // empty
}


// Converts byte to array of integers.
inline const array<uint8_t, 8> ILBMReader::ILBM::GetByteData(const uint8_t byte) const
{
	array<uint8_t, 8> arr;
	for (size_t n = 0; n < 8; ++n) {
		arr.at(7 - n) = (byte & (1 << n)) > 0 ? 1 : 0;
	}
	return arr;
}


// Planar lookup, by byte.
const array<uint8_t, 8> ILBMReader::ILBM::SumByteData(const vector<uint8_t>& bytes) const
{
	array<uint8_t, 8> result{ 0 };

	// Marching through the bytes in reverse order, we calculate 
	// an index for each bit position.
	for (auto b = rbegin(bytes); b != rend(bytes); ++b) {
		auto temp = GetByteData(*b);

		for (size_t n = 0; n < 8; ++n) {
			auto& stored_byte = result.at(n);
			stored_byte <<= 1;
			temp.at(n) |= stored_byte;
			result.at(n) = temp.at(n);
		}
	}
	return result;
}


// Return 8 colors from a given set of bytes.
const array<ILBMReader::color, 8> ILBMReader::ILBM::DerivePixelsByBytes(const array<uint8_t, 8> bytes) const
{
	array<ILBMReader::color, 8> colors = { 0 };
	auto palette = GetPalette();
	for (size_t n = 0; n < bytes.size(); ++n) {
		colors.at(n) = palette.at(bytes.at(n));
	}
	return colors;
}


const array<ILBMReader::color, 8> ILBMReader::ILBM::GetColorByte(const unsigned int position) const 
{
	const auto header = GetHeader();
	const auto bitplanes = header.GetBitplanesCount();
	const auto width_offset = header.GetWidth() / 8;
	const auto& data = extracted_bitplanes_;

	const auto bitplane_zero_pos = position + 
		((bitplanes-1) * width_offset) * (position / (width_offset)); // Offset due to the other bitplanes.

	vector<uint8_t> bytes;
	for (size_t n = 0; n < bitplanes; ++n) {
		const auto scanline_pos = bitplane_zero_pos + (n * width_offset);
		bytes.emplace_back(data.at(scanline_pos));
	}

	return DerivePixelsByBytes(SumByteData(bytes));	
}

// Rebuild this to output a vector of pixel objects. Those are structs; x, y, r, g, b.
// Build that as we decode the stream.
// Then, put it inside the PixelData object, which also provides width, height and other info,
// and which is iterable.
const vector<ILBMReader::pixel> ILBMReader::ILBM::GetImage() const
{
	const auto header = GetHeader();
	const auto height = header.GetHeight();
	const auto width = header.GetWidth();

	// We assign this as one massive allocation rather than many.
	const auto number_of_pixels = width * height;
	vector<ILBMReader::pixel> raster_lines(number_of_pixels);

	unsigned int absolute_position = 0;

	uint16_t x = 0;
	uint16_t y = 0;
	for (int position = 0; position < (width * height) / 8; position++) {
		auto colors = GetColorByte(position);
		for (size_t i = 0; i < 8; ++i) {
			auto& col = colors.at(i);
			raster_lines.at(absolute_position++) = pixel({ x, y, col.r, col.g, col.b });
			++x;
			if (x >= width) {
				x = 0;
				y++;
			}
		}
	}

	return raster_lines;
}



// ILBM consists of multiple chunks, fabricated here.
inline ILBMReader::ILBM::ILBM(bytestream& stream)
{
	ChunkFactory(stream);
	ComputeInterleavedBitplanes();
}


// General ILBM data. All valid ILBM files have a HEAD chunk. If not 
// found, return empty HEAD.
const ILBMReader::BMHD ILBMReader::ILBM::GetHeader() const
{
	const auto found_chunk = chunks_.find(CHUNK_T::BMHD);

	return (found_chunk != chunks_.end()) ?
		dynamic_cast<ILBMReader::BMHD&> (*found_chunk->second.get()) :
		BMHD();
}


// All valid ILBM files have a CMAP chunk. This holds color data.
// ILBM format permits a full byte per component in r g b, but OCS 
// cannot display 8 bit color; well-formed OCS images repeat the high nibble 
// for every component for the purposes of IFF files.
const vector<ILBMReader::color> ILBMReader::ILBM::GetPalette() const
{
	const auto found_chunk = chunks_.find(CHUNK_T::CMAP);

	return (found_chunk != chunks_.end()) ? 
		dynamic_cast<ILBMReader::CMAP&>(*found_chunk->second.get()).GetPalette() : 
		vector<color>();
}


const vector<uint8_t> ILBMReader::ILBM::FetchData(const bool compressed) const
{
	const auto found_chunk = chunks_.find(CHUNK_T::BODY);
	if (found_chunk == chunks_.end()) {
		return vector<uint8_t>();
	}

	auto ref = dynamic_cast<ILBMReader::BODY&>(*found_chunk->second.get());
	return (compressed) ? ref.GetUnpacked_ByteRun1() : ref.GetRawData();
}

void ILBMReader::ILBM::ComputeInterleavedBitplanes()
{
	if (extracted_bitplanes_.size() == 0) {
		const auto compression = GetHeader().Compression();
		const auto found_chunk = chunks_.find(CHUNK_T::BMHD);
		const bool is_compressed = ((found_chunk != chunks_.end()) && compression != 0) ? 1 : 0;
		extracted_bitplanes_ = FetchData(is_compressed);
	}
}


ILBMReader::FORM::FORM(bytestream& stream) 
{
	size_ = read_long(stream);

	const auto tag = read_tag(stream);

	if (tag == "ILBM") {
		form_contents_ = make_shared<ILBM>(ILBM(stream));
	}
}


shared_ptr<ILBMReader::ILBM> ILBMReader::FORM::Get_ILBM() const
{
	return form_contents_;
}


// This will need enhancement and error checking.
ILBMReader::File::File(const string& path) 
{
	bytestream stream(path, std::ios::binary);

	auto tag = read_tag(stream);
	if (tag == "FORM") {
		file_contents_ = make_shared<FORM>(FORM(stream));
	}
}


const shared_ptr<ILBMReader::ILBM> ILBMReader::File::GetAsILBM() const
{
	return file_contents_->Get_ILBM();
}


inline ILBMReader::INVALID_FORM::INVALID_FORM(bytestream&) 
{
}
