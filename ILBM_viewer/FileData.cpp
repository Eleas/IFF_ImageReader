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


// Stores tag as string. Only implemented for UNKNOWN.
void IFFReader::CHUNK::AddTagLiteral(const string tag)
{
	// ... an identified chunk has a well-known name, so 
	//	we do nothing here (see UNKNOWN chunk).
}


inline const string IFFReader::read_tag(bytestream& stream)
{
	char temptag[4];
	stream.read(reinterpret_cast<uint8_t*>(temptag), 4);
	return string(temptag, sizeof(temptag) / sizeof(char));
}


// Reads big endian longword (4 bytes), returns little endian.
const uint32_t IFFReader::read_long(bytestream& stream)
{
	uint32_t buffer;
	stream.read(reinterpret_cast<uint8_t*>(&buffer), 4);
	uint32_t tmp = ((buffer << 8) & 0xFF00FF00) | ((buffer >> 8) & 0xFF00FF);
	buffer = (tmp << 16) | (tmp >> 16);
	return buffer;
}


// Reads big endian word (2 bytes), returns little endian.
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
inline IFFReader::CMAP::CMAP( bytestream& stream ) : CHUNK( stream ) 
{
	palette_.clear();
	const auto color_count = GetSize() / 3;	 // 3 bytes: R,G,B.
	palette_.resize(color_count);

	for (auto& color : palette_) {
		color = { 
			read_byte( stream ),
			read_byte( stream ),
			read_byte( stream ) 
		};
	}
}


const vector<IFFReader::color> IFFReader::CMAP::GetPalette( ) const 
{ 
	return palette_; 
}


inline IFFReader::CAMG::CAMG( ) 
{
}


inline IFFReader::CAMG::CAMG( bytestream& stream ) : CHUNK( stream ) 
{ 
	contents_ = read_long( stream ); 
}


inline IFFReader::DPI::DPI() 
{	
}


inline IFFReader::DPI::DPI( bytestream& stream ) : CHUNK( stream ) 
{ 
	contents_ = read_long( stream ); 
}


inline IFFReader::BODY::BODY()
{
}


// BODY data fetched from stream as raw bytes.
inline IFFReader::BODY::BODY( bytestream& stream ) : CHUNK( stream ) 
{
	for ( uint32_t i = 0; i < GetSize(); ++i ) {
		raw_data_.emplace_back( read_byte( stream ) );
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

	while ( position < original_size ) {
		const auto value = static_cast<int8_t>( raw_data_.at( position++ ) ); // Make it signed.

		/*
		
		[Some versions of Adobe Photoshop incorrectly use the n = 128 no - op as 
		 a repeat code, which breaks strictly conforming readers. To read Photoshop 
		 ILBMs, allow the use of n = 128 as a repeat.
		 
		 This is pretty safe, since no known program writes real no - ops into their ILBMs.
		 
		 The reason n = 128 is a no - op is historical : the Mac Packbits buffer was only 128 
		 bytes, and a repeat code of 128 generates 129 bytes.] 
		
		*/

		for ( int i = 0; i < abs(value) + 1; ++i ) {
			unpacked_data.emplace_back( raw_data_.at( position ) );
			if ( value >= 0 ) {	// positive: copy that many bytes + 1 straight from original. 
				++position;		// Negative: copy byte an equal number of times (ignoring the minus).
			}
		}
		if ( value < 0 ) {
			++position;
		}
	}

	return move( unpacked_data );
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

		// Chunk is identified and assigned tag.
		const auto found_chunk = supported_chunks_.find(tag);
		const auto found_tag =
			(found_chunk != supported_chunks_.end()) ? 
				found_chunk->second : 
				CHUNK_T::UNKNOWN;

		// If the chunk was unknown and unknown chunk exists, don't make new one.
		if ( found_tag != CHUNK_T::UNKNOWN || chunks_.find( CHUNK_T::UNKNOWN ) == chunks_.end() ) {
			chunks_[found_tag] = ChunkFactoryInternals(stream, found_tag);
		}

		// Log the tag literal.
		chunks_[found_tag]->AddTagLiteral(tag);

		// Special handling. No more data can exist after BODY tag, so terminate. 
		// That's a hack; better to ensure !stream.good() after parsing BODY, or 
		// to count the bytes.
		if (found_tag == CHUNK_T::BODY) { 
			return; 
		}
	}
}


// Fabricates appropriate chunk from stream. Saves explicit pointers for easy access.
shared_ptr<IFFReader::CHUNK> IFFReader::ILBM::ChunkFactoryInternals(bytestream& stream, 
	const CHUNK_T found_chunk)
{
	switch (found_chunk) {
		case CHUNK_T::BMHD:
			header_ = make_shared<BMHD>(BMHD(stream));
			return header_;
		case CHUNK_T::CMAP:
			cmap_ = make_shared<CMAP>(CMAP(stream));
			return cmap_;
		case CHUNK_T::CAMG:	
			camg_ = make_shared<CAMG>(CAMG(stream));
			return camg_;
		case CHUNK_T::DPI:		
			return move(make_shared<DPI>(DPI(stream)));
		case CHUNK_T::BODY:		
			body_ = make_shared<BODY>(BODY(stream));
			return body_;
		case CHUNK_T::UNKNOWN:	
		default:
			return move(make_shared<UNKNOWN>(UNKNOWN(stream)));
	}
}


// Computes one planar pixel to one chunky pixel.
const inline uint8_t PlanarToChunky( const std::vector<uint8_t>& bits, 
	const int x, 
	const int y, 
	const int width, 
	const int bitplanes ) 
{
	const auto scan_line_bytelength = (width/8) + 
		(width % 8 != 0 ? 1 : 0);	// Round up the scan line width to nearest byte.

	const auto raster_line_bytelength = scan_line_bytelength * bitplanes;
	const auto startbyte = ( y * raster_line_bytelength ) + (x / 8);
	const auto bitpos = 7 - (x % 8);	// we count from highest to lowest.

	uint8_t buffer = 0;
	uint8_t byte = 0;
	unsigned int bytepos = 0;

	for (uint8_t n = 0; n < bitplanes; ++n) {
		bytepos = startbyte + (n * scan_line_bytelength);
		byte = bits.at(bytepos);
		buffer |= ((1 << bitpos) & byte) != 0 ? 1 << n : 0;
	}

	return buffer;
}


const vector<IFFReader::pixel> IFFReader::ILBM::ComputeScreenValues() const
{
	// Pixel buffer is set as single allocation rather than many.
	const auto pixel_count = width() * height();
	vector<IFFReader::pixel> colors( pixel_count );

	unsigned int bit_position = 0;
	uint16_t x = 0;
	uint16_t y = 0;

	color col;

	while ( bit_position < pixel_count ) {
		col = GetPalette().at( PlanarToChunky (
			extracted_bitplanes_, 
			x, 
			y, 
			width(), 
			bitplanes_count() )
		);

		colors.at( bit_position++ ) = pixel { 
			x++, 
			y, 
			col.r, 
			col.g, 
			col.b 
		};

		if ( x >= width() ) {
			++y;
			x = 0;
		}
	}

	return move(colors);
}


// ILBM consists of multiple chunks, fabricated here.
inline IFFReader::ILBM::ILBM(bytestream& stream)
{
	ChunkFactory( stream );
	DetermineSpecialGraphicModes( ); // EHB, HAM, AGA..?
	ComputeInterleavedBitplanes( );
}


// ILBM graphics functions. Later, inherit this from Displayable API, allowing
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


// All valid ILBM files have a CMAP chunk for color data. ILBM format stores
// a full byte per component in rgb, but OCS cannot display 8 bit color.
// Well-formed OCS images instead repeat the high nibble for every component 
// for the purposes of IFF ILBM files. For example, $0055aa.
// 
// A few early IFF readers generated malformed CMAP chunks, recognized by 
// setting the high nibble to 0; thus $fff becomes $0f0f0f. Correct for this
// when possible.
const vector<IFFReader::color> IFFReader::ILBM::GetPalette() const
{
	if ( !cmap_ ) {
		return vector<color>();
	}

	return cmap_->GetPalette();
}


// Find lowest common denominator for image.
void IFFReader::ILBM::DetermineSpecialGraphicModes()
{
	// Start with EHB and HAM. First, find the CAMG.
}


const vector<uint8_t> IFFReader::ILBM::FetchData(const uint8_t compression_method) const
{
	if ( !body_ ) {
		return vector<uint8_t>();
	}

	switch (compression_method) {
		case 1:		return body_->GetUnpacked_ByteRun1( );
		default:	return body_->GetRawData( );
	}
}


void IFFReader::ILBM::ComputeInterleavedBitplanes()
{
	if ( extracted_bitplanes_.empty() ) {
		extracted_bitplanes_ = FetchData( header_->Compression() );
	}
	pixels_ = ComputeScreenValues();
}


IFFReader::File::File( const string& path ) : 
	path_( path ), size_( 0 ), type_( IFFReader::IFF_T::FORM_NOT_FOUND )
{
	stream_.open( path, std::ios::binary );
	
	if ( !stream_.is_open() ) {
		return;
	}

	try {
		if ( read_tag(stream_) != "FORM" ) {
			type_ = IFFReader::IFF_T::UNREADABLE;
			return;
		}

		size_ = read_long( stream_ );
		const auto tag = read_tag( stream_ );

		// Make more interesting later.
		if ( tag == "ILBM" ) {
			asILBM_ = shared_ptr<ILBM>( new ILBM(stream_) );
			type_ = IFFReader::IFF_T::ILBM;
		}
	}
	catch (...) {  // Filthy. Add proper exceptions later.
		type_ = IFFReader::IFF_T::UNREADABLE; // Abort if file malformed or missing.
	}
}


// Returns blank pointer if file was not parsed. Have fun with that.
shared_ptr<IFFReader::ILBM> IFFReader::File::AsILBM() const
{
	if ( type_ != IFFReader::IFF_T::ILBM ) {
		return shared_ptr< IFFReader::ILBM >();
	}
	return asILBM_;
}


const IFFReader::IFF_T IFFReader::File::GetType() const
{
	return type_; 
}
