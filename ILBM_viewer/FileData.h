#pragma once
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

using std::array;
using std::basic_ifstream;
using std::make_shared;
using std::make_unique;
using std::map;
using std::move;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;


namespace IFFReader {
	typedef basic_ifstream<uint8_t> bytestream;
	typedef vector<uint8_t> bytefield;

	const string read_tag(bytestream& stream);
	const uint32_t read_long(bytestream& stream);
	const uint16_t read_word(bytestream& stream);
	const uint8_t read_byte(bytestream& stream);

	// List of recognized IFF formats.
	enum class IFF_T { ILBM, UNKNOWN_FORMAT, UNREADABLE, FORM_NOT_FOUND };

	// List of recognized chunk types.
	enum class CHUNK_T { BMHD, CMAP, CAMG, DPI, BODY, UNKNOWN };

	// List of graphical modes.
	enum class GMODES_T { OCS, EHB, HAM6, SHAM, AGA, HAM8, TRUECOLOR };


	// Common chunk behavior.
	class CHUNK {
		uint32_t size_;

	public:
		CHUNK();
		CHUNK(bytestream& stream);
		virtual ~CHUNK();

		virtual const uint32_t GetSize() const;

		// Stores tag as as a string. Only implemented for UNKNOWN chunk.
		virtual void AddTagLiteral(const string tag);
	};


	// Instead of nulling them, set them in initializer.
	class BMHD : public CHUNK {
		uint16_t width_;
		uint16_t height_;
		uint16_t xcoordinate_;	 // X position (used in DPaint, etc)
		uint16_t ycoordinate_;	 // Y position (same)
		uint8_t bitplanes_;		 // # of bitplanes, sans mask
		uint8_t masking_;		 // Masking technique used. 0 = none, 1 = has mask, 2 = has transparent color, 3 = lasso
		uint8_t compression_;	 // Compression type used. 0 = none, 1 = byteRun1
		uint16_t transparency_;  // Transparent background color 
		uint8_t x_aspect_ratio_; // Horizontal pixel size 
		uint8_t y_aspect_ratio_; // Vertical pixel size 
		uint16_t page_width_;    // Horizontal resolution of display device 
		uint16_t page_height_;   // Vertical resolution of display device 

	public:
		BMHD();
		BMHD(bytestream& stream);

		const uint16_t GetWidth() const;
		const uint16_t GetHeight() const;
		const uint16_t GetBitplanesCount() const;
		const uint8_t Compression() const;
	};


	typedef struct color {
		uint8_t r; uint8_t g; uint8_t b;
	} color;


	typedef struct pixel {
		uint16_t x;
		uint16_t y;
		uint8_t r; 
		uint8_t g; 
		uint8_t b;
	} pixel;


	class CMAP : public CHUNK {
		vector<color> palette_;

	public:
		CMAP();
		CMAP(bytestream& stream);

		const vector<color> GetPalette() const;
	};


	// Optional or unimplemented chunks.
	class CAMG : public CHUNK {
		uint32_t contents_ = 0;

	public:
		CAMG();
		CAMG(bytestream& stream);
	};


	class DPI : public CHUNK {
		uint32_t contents_ = 0;

	public:
		DPI();
		DPI(bytestream& stream);
	};


	// Holds the bitfields for the image.
	class BODY : public CHUNK {
		bytefield raw_data_;

	public:
		BODY();
		BODY(bytestream& stream);

		// Use if compression bit is unset.
		const bytefield& GetRawData() const;

		// Use if compression bit is set.
		const bytefield GetUnpacked_ByteRun1() const;
	};


	// Represents chunks that are not identified, but still need handling.
	class UNKNOWN : public CHUNK {
		vector<string> unknown_chunk_names_;

	public:
		UNKNOWN();
		UNKNOWN(bytestream& stream);

		// Stores tag strings, creating a log list of unparsed ands skipped tags.
		void AddTagLiteral(const string tag) override;
	};


	class ILBM : public CHUNK {
	private:
		// Chunk map
		map<CHUNK_T, shared_ptr<CHUNK>> chunks_;

		// Extracted image data
		bytefield extracted_bitplanes_;
		vector<IFFReader::pixel> pixels_;

		// Chunk data
		shared_ptr<IFFReader::BMHD> header_;
		shared_ptr<IFFReader::CMAP> cmap_;
		shared_ptr<IFFReader::CAMG> camg_;
		shared_ptr<IFFReader::BODY> body_;

		// Constructs supported ILBM chunks from stream.
		void ChunkFactory(bytestream& stream);

		// Fabricates appropriate chunk from stream.
		shared_ptr<CHUNK> ChunkFactoryInternals(bytestream& stream, const CHUNK_T found_chunk);

		// All valid ILBM files have a CMAP chunk for color data. ILBM format stores
		// a full byte per component in rgb, but OCS cannot display 8 bit color.
		// Well-formed OCS images instead repeat the high nibble for every component 
		// for the purposes of IFF ILBM files. For example, $0055aa.
		// 
		// A few early IFF readers generated malformed CMAP chunks, recognized by 
		// setting the high nibble to 0; thus $fff becomes $0f0f0f. Correct for this
		// when possible.
		const vector<color> GetPalette() const;

		// Color handling
		void DetermineSpecialGraphicModes();

		// Loads data, computes screen values.
		void ComputeInterleavedBitplanes();

		// Loads data from BODY tag, decompressing as appropriate.
		inline const bytefield FetchData(const uint8_t compression_method) const;

		// Computes chunky pixel field, matching each pixel to palette value.
		const vector<IFFReader::pixel> ComputeScreenValues() const;

	public:
		ILBM(bytestream& stream);

		// ILBM graphics functions. Replace with Displayable API, allowing
		// all image formats to display in the same way.

		vector<IFFReader::pixel>::const_iterator begin();
		vector<IFFReader::pixel>::const_iterator end();
		const uint32_t width() const;
		const uint32_t height() const;
		const uint16_t bitplanes_count() const;
		
	};


	class File {
	private:
		string path_;
		IFF_T type_;
		bytestream stream_;
		uint32_t size_;
		shared_ptr<ILBM> asILBM_;

	public:
		File(const string& path);

		shared_ptr<ILBM> AsILBM() const;
		const IFFReader::IFF_T GetType() const;
	};
}