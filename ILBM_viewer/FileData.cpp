#include "FileData.h"


inline IFFReader::CHUNK::CHUNK() : size_(0) 
{ 
}


inline IFFReader::CHUNK::CHUNK(bytestream& stream) : size_(read_long(stream)) 
{
}


inline IFFReader::CHUNK::~CHUNK() 
{
}


inline const uint32_t IFFReader::CHUNK::GetSize() const 
{ 
	return size_; 
}


// Stores the tag as as a string. Only implemented for UNKNOWN.
void IFFReader::CHUNK::AddTagLiteral(const string tag)
{
	// ... an identified chunk has a well known name, so 
	//	we do nothing here (see UNKNOWN chunk).
}


inline const string IFFReader::read_tag(bytestream& stream)
{
	char temptag[4];
	stream.read(reinterpret_cast<uint8_t*>(temptag), 4);
	return string(temptag, sizeof(temptag) / sizeof(char));
}


// Reads big endian longword (4 bytes) and returns a little endian.
const uint32_t IFFReader::read_long(bytestream& stream)
{
	uint32_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 4);
	uint32_t tmp = ((buffer << 8) & 0xFF00FF00) | ((buffer >> 8) & 0xFF00FF);
	buffer = (tmp << 16) | (tmp >> 16);
	return buffer;
}


// Reads big endian word (2 bytes) and returns a little endian.
inline const uint16_t IFFReader::read_word(bytestream& stream) 
{
	uint16_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 2);
	return (buffer >> 8) | (buffer << 8);
}


// Reads byte.
inline const uint8_t IFFReader::read_byte(bytestream& stream) 
{
	uint8_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 1);
	return buffer;
}


IFFReader::BMHD::BMHD() : width_{ 0 }, height_{ 0 }, xcoordinate_{ 0 }, ycoordinate_{ 0 }, 
	bitplanes_{ 0 }, masking_{ 0 }, compression_{ 0 }, transparency_{ 0 }, x_aspect_ratio_{ 0 }, 
	y_aspect_ratio_{ 0 }, page_width_{ 0 }, page_height_{ 0 }
{ 
}


IFFReader::BMHD::BMHD(bytestream& stream) : CHUNK(stream) 
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


inline const uint16_t IFFReader::BMHD::GetWidth() const 
{ 
	return width_; 
}


inline const uint16_t IFFReader::BMHD::GetHeight() const 
{
	return height_; 
}


inline const uint16_t IFFReader::BMHD::GetBitplanesCount() const 
{ 
	return bitplanes_; 
}


const uint8_t IFFReader::BMHD::Compression() const
{
	return compression_;
}


inline IFFReader::CMAP::CMAP() 
{
}


// Extracts palette as a collection of colors.
inline IFFReader::CMAP::CMAP(bytestream& stream) : CHUNK(stream) 
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


const vector<IFFReader::color> IFFReader::CMAP::GetPalette() const 
{ 
	return palette_; 
}


inline IFFReader::CAMG::CAMG() 
{
}


inline IFFReader::CAMG::CAMG(bytestream& stream) : CHUNK(stream) 
{ 
	contents_ = read_long(stream); 
}


inline IFFReader::DPI::DPI() 
{	
}


inline IFFReader::DPI::DPI(bytestream& stream) : CHUNK(stream) 
{ 
	contents_ = read_long(stream); 
}


inline IFFReader::BODY::BODY()
{
}


// BODY data fetched from stream as raw bytes.
inline IFFReader::BODY::BODY(bytestream& stream) : CHUNK(stream) 
{
	for (uint32_t i = 0; i < GetSize(); ++i) {
		raw_data_.emplace_back(read_byte(stream));
	}
}


const vector<uint8_t>& IFFReader::BODY::GetRawData() const
{
	return raw_data_;
}


// Unpacks raw data using ByteRun1 encoding. 
//	[http://amigadev.elowar.com/read/ADCD_2.1/Devices_Manual_guide/node01C0.html]
const vector<uint8_t> IFFReader::BODY::GetUnpacked_ByteRun1() const
{
	const auto original_size = raw_data_.size();
	
	vector<uint8_t> unpacked_data; // Destination.
	size_t position = 0;

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
IFFReader::UNKNOWN::UNKNOWN()
{
}


// Unknown tags are bypassed without extracting data.
inline IFFReader::UNKNOWN::UNKNOWN(bytestream& stream) : CHUNK(stream)
{
	stream.ignore(GetSize()); // Sod off, on to next chunk with ye.
}


// Logs the tag literal of unknown chunks.
void IFFReader::UNKNOWN::AddTagLiteral(const string name)
{
	unknown_chunk_names_.push_back(name);
}


// Detects chunk type, fabricates. Unknown chunks beyond the first are logged.
void IFFReader::ILBM::ChunkFactory(bytestream& stream) 
{
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
shared_ptr<IFFReader::CHUNK> IFFReader::ILBM::ChunkFactoryInternals(bytestream& stream, const CHUNK_T found_chunk)
{
	switch (found_chunk) {
		case CHUNK_T::BMHD:
			header_ = make_shared<BMHD>(BMHD(stream));
			return header_;
		case CHUNK_T::CMAP:
			return move(make_shared<CMAP>(CMAP(stream)));
		case CHUNK_T::CAMG:		return move(make_shared<CAMG>(CAMG(stream)));
		case CHUNK_T::DPI:		return move(make_shared<DPI>(DPI(stream)));
		case CHUNK_T::BODY:		return move(make_shared<BODY>(BODY(stream)));
		case CHUNK_T::UNKNOWN:	
		default:
			return move(make_shared<UNKNOWN>(UNKNOWN(stream)));
	}
}


// Converts byte to array of integers.
inline const array<uint8_t, 8> IFFReader::ILBM::GetByteData(const uint8_t byte) const
{
	array<uint8_t, 8> arr;
	for (size_t n = 0; n < 8; ++n) {
		arr.at(7 - n) = (byte & (1 << n)) > 0 ? 1 : 0;
	}
	return arr;
}


// Planar lookup, by byte.
const array<uint8_t, 8> IFFReader::ILBM::SumByteData(const vector<uint8_t>& bytes) const
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
const array<IFFReader::color, 8> IFFReader::ILBM::DerivePixelsByBytes(const array<uint8_t, 8> bytes) const
{
	array<IFFReader::color, 8> colors = { 0 };
	auto palette = GetPalette();
	for (size_t n = 0; n < bytes.size(); ++n) {
		colors.at(n) = palette.at(bytes.at(n));
	}
	return colors;
}


// Planar to chunky conversion, 8 bits at a time.
const array<IFFReader::color, 8> IFFReader::ILBM::GetColorByte(const unsigned int position) const 
{
	const auto width_offset = width() / 8;
	const auto& data = extracted_bitplanes_;

	const auto bitplane_zero_pos = position + 
		((bitplanes_count() -1) * width_offset) * (position / (width_offset)); // Offset due to the other bitplanes.

	vector<uint8_t> bytes;
	for (size_t n = 0; n < bitplanes_count(); ++n) {
		const auto scanline_pos = bitplane_zero_pos + (n * width_offset);
		bytes.emplace_back(data.at(scanline_pos));
	}

	return DerivePixelsByBytes(SumByteData(bytes));	
}


// Rebuild this to output a vector of pixel objects. Those are structs; x, y, r, g, b.
// Build that as we decode the stream.
// Then, put it inside the PixelData object, which also provides width, height and other info,
// and which is iterable.
const vector<IFFReader::pixel> IFFReader::ILBM::ComputePlanarToChunky() const
{
	// We assign this as one massive allocation rather than many.
	const auto number_of_pixels = width() * height();
	vector<IFFReader::pixel> raster_lines(number_of_pixels);

	unsigned int absolute_position = 0;

	uint16_t x = 0;
	uint16_t y = 0;
	for (unsigned int position = 0; position < (number_of_pixels) / 8; position++) {
		auto colors = GetColorByte(position);

		for (size_t i = 0; i < 8; ++i) {
			auto& col = colors.at(i);
			raster_lines.at(absolute_position++) = pixel({ x, y, col.r, col.g, col.b });
			++x;
			if (x >= width()) {
				x = 0;
				y++;
			}
		}
	}

	return raster_lines;
}


// ILBM consists of multiple chunks, fabricated here.
inline IFFReader::ILBM::ILBM(bytestream& stream)
{
	ChunkFactory(stream);
	ComputeInterleavedBitplanes();
}


// ILBM graphics functions. Later, we inherit this from Displayable API, allowing
// all image formats to display in the same way.
vector<IFFReader::pixel>::const_iterator IFFReader::ILBM::begin()
{
	return pixels_.begin();
}


vector<IFFReader::pixel>::const_iterator IFFReader::ILBM::end()
{
	return pixels_.end();
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


// All valid ILBM files have a CMAP chunk. This holds color data.
// ILBM format permits a full byte per component in r g b, but OCS 
// cannot display 8 bit color; well-formed OCS images repeat the high nibble 
// for every component for the purposes of IFF ILBM files.
// 
// Some early IFF readers have been known to have malformed CMAP chunks,
// recognizable by setting the high nibble to 0; thus $fff becomes $0f0f0f.
// We need to optionally correct for this tendency if possible.
const vector<IFFReader::color> IFFReader::ILBM::GetPalette() const
{
	const auto found_chunk = chunks_.find(CHUNK_T::CMAP);

	return (found_chunk != chunks_.end()) ? 
		dynamic_cast<IFFReader::CMAP&>(*found_chunk->second.get()).GetPalette() : 
		vector<color>();
}


const vector<uint8_t> IFFReader::ILBM::FetchData(const uint8_t compression_method) const
{
	const auto found_chunk = chunks_.find(CHUNK_T::BODY);
	if (found_chunk == chunks_.end()) {
		return vector<uint8_t>();
	}

	auto ref = dynamic_cast<IFFReader::BODY&>(*found_chunk->second.get());

	switch (compression_method) {
		case 1:		return ref.GetUnpacked_ByteRun1();
		default:	return ref.GetRawData();
	}
}


void IFFReader::ILBM::ComputeInterleavedBitplanes()
{
	if (extracted_bitplanes_.size() == 0) {
		const auto compression = header_->Compression();
		const bool is_compressed = compression != 0 ? 1 : 0;
		extracted_bitplanes_ = FetchData(is_compressed);
	}
	pixels_ = ComputePlanarToChunky();
}


IFFReader::File::File(const string& path) : path_(path), size_(0), type_(IFFReader::IFF_T::UNKNOWN_FORMAT)
{
	stream_.open(path, std::ios::binary);
	
	try {
		if (read_tag(stream_) != "FORM") {
			type_ = IFFReader::IFF_T::UNREADABLE;
			return;
		}

		size_ = read_long(stream_);
		auto tag = read_tag(stream_);

		// Make more interesting later.
		if (tag == "ILBM") {
			asILBM_ = shared_ptr<ILBM>(new ILBM(stream_));
			type_ = IFFReader::IFF_T::ILBM;
		}
	}
	catch (...) {  // Yes, filthy. Point is to simply abort if file is malformed or missing.
		type_ = IFFReader::IFF_T::UNREADABLE;
	}
}


shared_ptr<IFFReader::ILBM> IFFReader::File::AsILBM() const 
{
	if (type_ != IFFReader::IFF_T::ILBM) 
		return shared_ptr<IFFReader::ILBM>(); // Returns blank pointer if file was not parsed. Have fun with that.

	return asILBM_;
}


const IFFReader::IFF_T IFFReader::File::GetType() const
{
	return type_; 
}
