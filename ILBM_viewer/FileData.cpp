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

inline const string ILBMReader::read_tag(bytestream& stream)
{
	char temptag[4];
	stream.read(reinterpret_cast<uint8_t*>(temptag), 4);
	return string(temptag, sizeof(temptag) / sizeof(char));
}

inline const uint32_t ILBMReader::read_long(bytestream& stream)
{
	uint32_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 4);
	uint32_t tmp = ((buffer << 8) & 0xFF00FF00) | ((buffer >> 8) & 0xFF00FF);
	buffer = (tmp << 16) | (tmp >> 16);
	return buffer;
}

inline const uint16_t ILBMReader::read_word(bytestream& stream) 
{
	uint16_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 2);
	return (buffer >> 8) | (buffer << 8);
}

inline const uint8_t ILBMReader::read_byte(bytestream& stream) 
{
	uint8_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 1);
	return buffer;
}

inline ILBMReader::BMHD::BMHD()
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

inline ILBMReader::CMAP::CMAP() {}

inline ILBMReader::CMAP::CMAP(bytestream& stream) : CHUNK(stream) 
{
	palette_.clear();
	const auto color_count = GetSize() / 3;
	palette_.resize(color_count);

	for (auto& color : palette_) {
		color = { read_byte(stream),
			read_byte(stream),
			read_byte(stream) };
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

/*
Appendix D. ByteRun1 Run Encoding
The run encoding scheme byteRun1 is best described by psuedo code for the decoder Unpacker (called UnPackBits in the Macintosh toolbox):

   UnPacker:
	  LOOP until produced the desired number of bytes
		 Read the next source byte into n
		 SELECT n FROM
			[0..127] => copy the next n+1 bytes literally
			[-1..-127]  => replicate the next byte -n+1 times
			-128  => no operation
			ENDCASE;
		 ENDLOOP;

..

Remember that each row of each scan line of a raster is separately packed.

[Some versions of Adobe Photoshop incorrectly use the n=128 no-op as a repeat code, which breaks
strictly conforming readers. To read Photoshop ILBMs, allow the use of n=128 as a repeat. This
is pretty safe, since no known program writes real no-ops into their ILBMs. The reason n=128 is
a no-op is historical: the Mac Packbits buffer was only 128 bytes, and a repeat code of 128
generates 129 bytes.]
*/



inline ILBMReader::BODY::BODY()
{
}

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

const vector<uint8_t> ILBMReader::BODY::GetUnpackedData() const
{
	vector<uint8_t> unpacked_data;
	const auto original_size = raw_data_.size();

	size_t iterator = 0;

	while (iterator < original_size) {
		const auto value = static_cast<int8_t>(raw_data_.at(iterator++));
		if (value == -128) continue;
		if (value >= 0) {
			for (int i = 0; i < value + 1; ++i) {
				unpacked_data.push_back(raw_data_.at(iterator++));
			}
		}
		if (value < 0) {
			for (int i = 0; i < (-value) + 1; ++i) {
				unpacked_data.push_back(raw_data_.at(iterator));
			}
			++iterator;
		}
	}

	return move(unpacked_data);
}

inline ILBMReader::UNKNOWN_CHUNK::UNKNOWN_CHUNK(bytestream& stream) : CHUNK(stream) 
{
}

void ILBMReader::ILBM::ChunkFactory(bytestream& stream) 
{
	while (stream.good()) {
		const auto found_chunk = supported_chunks_.find(read_tag(stream));

		// Replace this with check for unknown chunk object, so we can use it to skip past unimplemented chunk types.
		if (found_chunk == supported_chunks_.end()) {
			return;
		}

		chunks_[found_chunk->second] = ChunkFactoryInternals(stream, found_chunk->second);
	}
}

// From a stream, constructs a chunk.

unique_ptr<ILBMReader::CHUNK> ILBMReader::ILBM::ChunkFactoryInternals(bytestream& stream, const CHUNK_T found_chunk) const
{
	unique_ptr<CHUNK> result;

	switch (found_chunk) {
		case CHUNK_T::BMHD:		return move(make_unique<BMHD>(BMHD(stream)));
		case CHUNK_T::CMAP:		return move(make_unique<CMAP>(CMAP(stream)));
		case CHUNK_T::CAMG:		return move(make_unique<CAMG>(CAMG(stream)));
		case CHUNK_T::DPI:		return move(make_unique<DPI>(DPI(stream)));
		case CHUNK_T::BODY:		return move(make_unique<BODY>(BODY(stream)));
	}

	return result; // empty
}

inline ILBMReader::ILBM::ILBM(bytestream& stream)
{
	ChunkFactory(stream);
}

const ILBMReader::BMHD ILBMReader::ILBM::GetHeader() const
{
	const auto found_chunk = chunks_.find(CHUNK_T::BMHD);

	return (found_chunk != chunks_.end()) ?
		dynamic_cast<ILBMReader::BMHD&> (*found_chunk->second.get()) :
		BMHD();
}

// There's gotta be a better way to propagate data.
const vector<ILBMReader::color> ILBMReader::ILBM::GetPalette() const
{
	const auto found_chunk = chunks_.find(CHUNK_T::CMAP);

	return (found_chunk != chunks_.end()) ? 
		dynamic_cast<ILBMReader::CMAP&>(*found_chunk->second.get()).GetPalette() : 
		vector<color>();
}

const vector<uint8_t> ILBMReader::ILBM::GetBitData() const
{
	const auto found_chunk = chunks_.find(CHUNK_T::BODY);

	return (found_chunk != chunks_.end())?
		dynamic_cast<ILBMReader::BODY&>(*found_chunk->second.get()).GetUnpackedData() :
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

ILBMReader::File::File(const string& path) {
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
