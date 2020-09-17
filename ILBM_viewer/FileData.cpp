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
inline const uint32_t ILBMReader::read_long(bytestream& stream)
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


inline const uint16_t ILBMReader::BMHD::GetBitplanes() const 
{ 
	return bitplanes_; 
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
		raw_data_.push_back(read_byte(stream));
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
				unpacked_data.push_back(raw_data_.at(position++));
			}
		} else {
			for (int i = 0; i < (-value) + 1; ++i) {
				unpacked_data.push_back(raw_data_.at(position));
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
void ILBMReader::UNKNOWN::AddTagLiteral(const string tag)
{
	unknown_chunk_names_.push_back(tag);
}


// Detects chunk type, fabricates. Unknown chunks beyond the first are logged.
void ILBMReader::ILBM::ChunkFactory(bytestream& stream) 
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
	for (unsigned int n = 0; n < 8; ++n) {
		arr.at(7 - n) = (byte & (1 << n)) > 0 ? 1 : 0;
	}
	return arr;
}


// Planar lookup, by byte.
const array<uint8_t, 8> ILBMReader::ILBM::SumByteData(const vector<uint8_t> bytes) const
{
	array<uint8_t, 8> result{ 0 };

	// Marching through the bytes in reverse order, we calculate 
	// an index for each bit position.
	for (auto b = rbegin(bytes); b != rend(bytes); ++b) {
		auto temp = GetByteData(*b);

		for (int n = 0; n < 8; ++n) {
			auto& stored_byte = result.at(n);
			stored_byte <<= 1;
			temp.at(n) |= stored_byte;
			result.at(n) = temp.at(n);
		}
	}
	return result;
}


// ILBM consists of multiple chunks, fabricated here.
inline ILBMReader::ILBM::ILBM(bytestream& stream)
{
	ChunkFactory(stream);
	auto b = SumByteData({ 0b00001111, 0b00001111, 0b00011000});
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


const vector<uint8_t> ILBMReader::ILBM::GetData() const
{
	const auto found_chunk = chunks_.find(CHUNK_T::BODY);

	return (found_chunk != chunks_.end())?
		dynamic_cast<ILBMReader::BODY&>(*found_chunk->second.get()).GetUnpacked_ByteRun1() :
		vector<uint8_t>();
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
