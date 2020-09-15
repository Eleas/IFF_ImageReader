#include "FileData.h"
#include "Reader.h"

inline IFFImageReader::CHUNK::CHUNK() : size_(0) 
{ 
}

inline IFFImageReader::CHUNK::CHUNK(bytestream& stream) : size_(read_long(stream)) 
{
}

inline IFFImageReader::CHUNK::~CHUNK() 
{
}

inline const uint32_t IFFImageReader::CHUNK::GetSize() const 
{ 
	return size_; 
}

inline IFFImageReader::BMHD::BMHD()
{ 
}

inline IFFImageReader::BMHD::BMHD(bytestream& stream) : CHUNK(stream) 
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

inline IFFImageReader::CMAP::CMAP() {}

inline IFFImageReader::CMAP::CMAP(bytestream& stream) : CHUNK(stream) 
{
	palette_.clear();
	const auto color_count = GetSize() / 3;
	palette_.resize(color_count);

	for (auto& color : palette_) {
		color = { read_byte(stream),
			read_byte(stream),
			read_byte(stream) };
	}

	//for (unsigned int i = 0; i < color_count; ++i) {
	//	palette_.push_back({
	//		read_byte(stream),
	//		read_byte(stream),
	//		read_byte(stream)
	//		});
	//}
}

const vector<IFFImageReader::color> IFFImageReader::CMAP::GetPalette() const 
{ 
	return palette_; 
}

inline IFFImageReader::CAMG::CAMG() 
{
}

inline IFFImageReader::CAMG::CAMG(bytestream& stream) : CHUNK(stream) 
{ 
	contents_ = read_long(stream); 
}

inline IFFImageReader::DPI::DPI() 
{	
}

inline IFFImageReader::DPI::DPI(bytestream& stream) : CHUNK(stream) 
{ 
	contents_ = read_long(stream); 
}

inline IFFImageReader::BODY::BODY() 
{
}

inline IFFImageReader::BODY::BODY(bytestream& stream) : CHUNK(stream) 
{
	for (uint32_t i = 0; i < GetSize(); ++i) {
		raw_data_.push_back(read_byte(stream));
	}
}

inline IFFImageReader::UNKNOWN_CHUNK::UNKNOWN_CHUNK(bytestream& stream) : CHUNK(stream) 
{
}

void IFFImageReader::ILBM::ChunkFactory(bytestream& stream) 
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

unique_ptr<IFFImageReader::CHUNK> IFFImageReader::ILBM::ChunkFactoryInternals(bytestream& stream, const CHUNK_T found_chunk) const
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

inline IFFImageReader::ILBM::ILBM(bytestream& stream)
{
	ChunkFactory(stream);
}

// There's gotta be a better way to propagate data.
const vector<IFFImageReader::color> IFFImageReader::ILBM::GetPalette() const
{
	const auto found_chunk = chunks_.find(CHUNK_T::CMAP);

	return (found_chunk != chunks_.end()) ? 
		dynamic_cast<IFFImageReader::CMAP&>(*found_chunk->second.get()).GetPalette() : 
		vector<color>();
}





IFFImageReader::FORM_CONTENTS::FORM_CONTENTS() { }
IFFImageReader::FORM_CONTENTS::~FORM_CONTENTS() {}

// Incomplete. If we want to expand the reader, it's no longer an IFF reader. To do:
// extract FORM from ILBM class.
inline unique_ptr<IFFImageReader::FORM_CONTENTS> IFFImageReader::FORM::FormFactory(bytestream& stream)
{
	const auto tag = read_tag(stream);

	if (tag == "ILBM") return move(make_unique<ILBM>(ILBM(stream)));
	return move(make_unique<INVALID_FORM>(INVALID_FORM(stream)));
}

IFFImageReader::FORM::FORM(bytestream& stream) 
{
	tag_ = read_tag(stream);
	size_ = read_long(stream);

	form_contents_ = move(FormFactory(stream));
}

inline IFFImageReader::INVALID_FORM::INVALID_FORM(bytestream&) {}
