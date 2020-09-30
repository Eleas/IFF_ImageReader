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
	typedef vector<color> colors;

	struct OCSmodes {
		uint32_t contents;
		bool HoldAndModify = false;
		bool ExtraHalfBrite = false;
		bool GenlockVideo = false;
		bool GenlockAudio = false;
		bool Interlace = false;
		bool Hires = false;			// Cannot be combined with SuperHires
		bool SuperHires = false;	// Cannot be combined with Hires
		bool Sprites = false;		// Using sprites, so sprite palettes will be loaded
		bool DualPlayfield = false;
		bool DualPlayfieldBAPriority = false;
		bool ViewportHide = false;	// Whether or not to hide the viewport
		bool DoubleScan = false;    // Uses double scan rate

		OCSmodes(uint32_t contents);
		/*
		"The DUALPF and PFBA modes are related. DUALPF tells the system to treat the
		raster specified by this ViewPort as the first of two independent and
		separately controllable playfields. It also modifies the manner in which
		the pixel colors are selected for this raster (see the above table).

		When PFBA is specified, it indicates that the second playfield has video
		priority over the first one. Playfield relative priorities can be controlled
		when the playfield is split into two overlapping regions. Single-playfield
		and dual-playfield modes are discussed in “Advanced Topics” below."

				[http://amigadev.elowar.com/read/ADCD_2.1/Libraries_Manual_guide/node0327.html]
		*/
	};


	class CMAP : public CHUNK {
		colors palette_;

	public:
		CMAP();
		CMAP(bytestream& stream);

		const colors GetPalette( const OCSmodes& mode) const;
	};


	// Amiga-specific image data chunk.
	class CAMG : public CHUNK {
		OCSmodes contents_ { 0 };

	public:
		CAMG();
		CAMG(bytestream& stream);

		// This describes Amiga specific modes, such as HAM, AGA, EHB etc.
		const OCSmodes GetModes() const;
	};


	// Chunk consists of one 32 bit field, not yet implemented.
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
	};



	class ILBM : public CHUNK {
	private:
		// Chunk map
		map<CHUNK_T, shared_ptr<CHUNK>> chunks_;
		map<string, shared_ptr<CHUNK>> unknown_chunks;

		// Extracted image data
		bytefield extracted_bitplanes_;

		// replacement for pixels vector
		vector<uint8_t> screen_data_;
		colors stored_palette_;

		// Chunk data
		shared_ptr<BMHD> header_;
		shared_ptr<CMAP> cmap_;
		shared_ptr<CAMG> camg_;
		shared_ptr<BODY> body_;

		// Constructs supported ILBM chunks from stream.
		void FabricateChunks(bytestream& stream);

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
		const colors GetPalette() const;

		// Loads data, computes screen values.
		void ComputeInterleavedBitplanes();

		// Loads data from BODY tag, decompressing as appropriate.
		inline const bytefield FetchData(const uint8_t compression_method) const;

		// Translates bitplanes into chunky indices
		const vector<uint8_t> ComputeScreenData() const;

	public:
		ILBM(bytestream& stream);

		// ILBM graphics functions. Replace with Displayable API, allowing
		// all image formats to display in the same way.

		const uint32_t width() const;
		const uint32_t height() const;
		const uint16_t bitplanes_count() const;

		// 
		const color at(unsigned int x, unsigned int y);

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
		const IFF_T GetType() const;
	};
}