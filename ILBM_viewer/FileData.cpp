#include "FileData.h"

typedef vector<uint8_t> bytefield;
typedef vector<IFFReader::color> colors;


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


IFFReader::BMHD::BMHD() : width_{ 0 }, height_{ 0 }, xcoordinate_{ 0 }, 
	ycoordinate_{ 0 }, 	bitplanes_{ 0 }, masking_{ 0 }, compression_{ 0 }, 
	transparency_{ 0 }, x_aspect_ratio_{ 0 }, 	y_aspect_ratio_{ 0 }, 
	page_width_{ 0 }, page_height_{ 0 }
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


const colors IFFReader::CMAP::GetPalette(const OCSmodes& mode) const
{ 
	if (mode.ExtraHalfBrite) {
		auto extended_palette = palette_;
		for (auto& col : palette_) {
			extended_palette.push_back(color{ uint8_t(col.r >> 1), uint8_t(col.b >> 1), uint8_t(col.g >> 1) });
		}
		return extended_palette;
	}
	return palette_; 
}


inline IFFReader::CAMG::CAMG( ) 
{
}


inline IFFReader::CAMG::CAMG( bytestream& stream ) : CHUNK( stream ) 
{ 
	contents_ = OCSmodes(read_long( stream )); // Parse various screen modes.
}


/*

There are several CAMG values saved by old ILBM writers which are invalid
modeID values.  An ILBM writer will produce ILBMs with invalid modeID
values if it:

	*  fails to mask out SPRITES|VP_HIDE|GENLOCK_AUDIO|GENLOCK_VIDEO.

	*  saves a 2.0 extended view mode as 16 bits rather than saving the
	   32-bit modeID returned by GetVPModeID().

	*  saves garbage in the upper word of the
	   CAMG value.

All valid modeIDs either have an upper word of 0 and do not have the
<graphics/view.h> EXTENDED_MODE bit set in the low word, or have a non-0
upper word and do have the EXTENDED_MODE bit set in the lower word.  CAMG
values which are invalid modeIDs must be screened out and fixed before
using them.
*/

// Check for bad CAMG and compensate.

// Create struct of flags.

/*	#define CAMG_HAM 0x800    hold and modify
	#define CAMG_EHB 0x80    extra halfbrite
	#define GENLOCK_VIDEO	0x0002
	#define LACE		0x0004
	#define DOUBLESCAN	0x0008
	#define SUPERHIRES	0x0020
	#define PFBA		0x0040
	#define EXTRA_HALFBRITE 0x0080
	#define GENLOCK_AUDIO	0x0100
	#define DUALPF		0x0400
	#define HAM		0x0800
	#define EXTENDED_MODE	0x1000
	#define VP_HIDE	0x2000
	#define SPRITES	0x4000
	#define HIRES		0x8000

	EXTENDED_MODE | SPRITES | VP_HIDE | GENLOCK_AUDIO | GENLOCK_VIDEO (=0x7102, mask=0x8EFD)
*/


// Various flags for parsing OCS modes
constexpr uint32_t GENLOCK_VIDEO = 0x0002;  //
constexpr uint32_t LACE = 0x0004;  //
constexpr uint32_t DOUBLESCAN = 0x0008;
constexpr uint32_t SUPERHIRES = 0x0020;
constexpr uint32_t PFBA = 0x0040;
constexpr uint32_t EXTRA_HALFBRITE = 0x0080;
constexpr uint32_t GENLOCK_AUDIO = 0x0100;
constexpr uint32_t DUALPF = 0x0400;
constexpr uint32_t HAM	= 0x0800;
constexpr uint32_t EXTENDED_MODE = 0x1000;
constexpr uint32_t VP_HIDE = 0x2000;
constexpr uint32_t SPRITES = 0x4000;
constexpr uint32_t HIRES = 0x8000;

inline IFFReader::OCSmodes::OCSmodes(uint32_t contents) : contents(contents) {
	Interlace				= (contents & LACE);
	DoubleScan				= (contents & DOUBLESCAN);
	ExtraHalfBrite			= (contents & EXTRA_HALFBRITE);
	HoldAndModify			= (contents & HAM);
	Hires					= (contents & HIRES);
	SuperHires				= (contents & SUPERHIRES);
	DualPlayfield			= (contents & DUALPF);
	DualPlayfieldBAPriority = (contents & PFBA);
}


// Express screen modes that only require CAMG to evaluate.
const IFFReader::OCSmodes IFFReader::CAMG::GetModes() const
{
	return contents_;
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


const bytefield& IFFReader::BODY::GetRawData() const
{
	return raw_data_;
}


// Unpacks raw data using ByteRun1 encoding. 
//[http://amigadev.elowar.com/read/ADCD_2.1/Devices_Manual_guide/node01C0.html]
const bytefield IFFReader::BODY::GetUnpacked_ByteRun1() const
{
	const auto original_size = raw_data_.size();
	
	bytefield unpacked_data; // Destination.
	size_t position = 0;
	int8_t value = 0;

	while ( position < (original_size -1)) { 
		value = static_cast<int8_t>(raw_data_.at(position++)); // Make it signed.
		/*

		[Some versions of Adobe Photoshop incorrectly use the n = 128 no - op as
			a repeat code, which breaks strictly conforming readers. To read Photoshop
			ILBMs, allow the use of n = 128 as a repeat.

			This is pretty safe, since no known program writes real no - ops into their ILBMs.

			The reason n = 128 is a no - op is historical : the Mac Packbits buffer was only 128
			bytes, and a repeat code of 128 generates 129 bytes.]

		*/

		for (int i = 0; i < abs(value) + 1; ++i) {
			unpacked_data.emplace_back(position < original_size ? // Prevent vector out of range.
				raw_data_.at(position) : 
				0); 

			if (value >= 0) {	// positive: copy that many bytes + 1 straight from original. 
				++position;		// Negative: copy byte an equal number of times (ignoring the minus).
			}
		}
		if (value < 0) {
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
void IFFReader::ILBM::FabricateChunks(bytestream& stream) 
{
	while (stream.good()) {
		const auto tag = read_tag(stream);

		// This describes list of available chunks.
		const map <string, CHUNK_T> supported_chunks_ = {
			{ "BMHD", CHUNK_T::BMHD },
			{ "CMAP", CHUNK_T::CMAP },
			{ "CAMG", CHUNK_T::CAMG },
			{ "DPI ", CHUNK_T::DPI },
			{ "BODY", CHUNK_T::BODY }
		};

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
const inline uint8_t PlanarToChunky_old( const std::vector<uint8_t>& bits, 
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


// Computes one planar pixel to one chunky pixel.
const inline uint8_t PlanarToChunky(const std::vector<uint8_t>& bits,
	const int absolute_position,
	const int width,
	const int bitplanes)
{
	const auto scan_line_bytelength = (width / 8) +
		(width % 8 != 0 ? 1 : 0);	// Round up the scan line width to nearest byte.

	const auto raster_line_bytelength = scan_line_bytelength * bitplanes;
	const auto startline = ((absolute_position/scan_line_bytelength/8)*raster_line_bytelength);
	const auto startbyte = startline + ((absolute_position/8)%scan_line_bytelength);
	const auto bitpos = 7 - (absolute_position%8);	// we count from highest to lowest.

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


// Possible idea: create a screen object to handle this.
const vector<uint8_t> IFFReader::ILBM::ComputeScreenData() const
{
	// Pixel buffer is set as single allocation rather than many.
	const auto pixel_count = width() * height();
	vector<uint8_t> data(pixel_count);

	unsigned int bit_position = 0;
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


// ILBM consists of multiple chunks, fabricated here.
IFFReader::ILBM::ILBM(bytestream& stream)
{
	try {
		FabricateChunks(stream);
	}
	catch (std::exception e) {
		throw iff_reading_error();
	}

	if (header_ == nullptr) {
		throw iff_bodiless_error();
	}
	if (body_ == nullptr) {
		throw iff_bodiless_error();
	}

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


const IFFReader::color IFFReader::ILBM::at(unsigned int x, unsigned int y)
{
	const auto position = static_cast<uint32_t>(y) * width() + static_cast<uint32_t>(x);
	return stored_palette_.at(screen_data_.at(position));
}


// Perhaps an update palette function would be good, 
// and then a simple getter instead.
const colors IFFReader::ILBM::GetPalette() const
{
	if (!cmap_) {
		return colors();
	}

	return cmap_->GetPalette((camg_) ? camg_->GetModes() : OCSmodes{ 0 });
}


// Find lowest common denominator for image.
void IFFReader::ILBM::DetermineSpecialGraphicModes()
{
	// Start with EHB and HAM. First, find the CAMG.
}


const bytefield IFFReader::ILBM::FetchData(const uint8_t compression) const
{
	if (!body_) {
		return bytefield();
	}

	switch (compression) {
	case 1:		return body_->GetUnpacked_ByteRun1();
	default:	return body_->GetRawData();
	}
}


void IFFReader::ILBM::ComputeInterleavedBitplanes()
{
	if (extracted_bitplanes_.empty()) {
		extracted_bitplanes_ = FetchData(header_->Compression());
	}
	screen_data_ = ComputeScreenData();
}



IFFReader::File::File(const string& path) :
	path_(path), size_(0), type_(IFFReader::IFF_T::UNKNOWN_FORMAT)
{
	stream_.open(path, std::ios::binary);

	if (!stream_.is_open()) {
		return;
	}

	try {
		if (read_tag(stream_) != "FORM" ) {
			return;
		}

		size_ = read_long( stream_ );
		const auto tag = read_tag( stream_ );

		if ( tag == "ILBM" ) {
			asILBM_ = shared_ptr<ILBM>( new ILBM(stream_) );
			type_ = IFFReader::IFF_T::ILBM;
		}
	}
	catch (iff_bad_or_mangled_error err) {  
		type_ = IFFReader::IFF_T::UNKNOWN_FORMAT; // Abort if file malformed or missing.
		throw err; // Instead of passing error on, consider handling it here.
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
