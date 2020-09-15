#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

using std::basic_ifstream;
using std::make_unique;
using std::map;
using std::move;
using std::string;
using std::unique_ptr;
using std::vector;


namespace IFFImageReader {
	typedef basic_ifstream<uint8_t> bytestream;

	// Common behavior (size) and polymorphism
	class CHUNK {
		uint32_t size_;
	public:
		CHUNK();
		CHUNK(bytestream& stream);
		virtual ~CHUNK();
		virtual const uint32_t GetSize() const;
	};


	// Stub, but useful if we want to expand from ILBM to other stuff
	class FORM_CONTENTS {
	public:
		FORM_CONTENTS();
		~FORM_CONTENTS();
	};

	// Instead of nulling them, set them in initializer.
	class BMHD : public CHUNK {
		uint16_t width_ = 0;
		uint16_t height_ = 0;
		uint16_t xcoordinate_ = 0;
		uint16_t ycoordinate_ = 0;
		uint8_t bitplanes_ = 0;
		uint8_t masking_ = 0;
		uint8_t compression_ = 0;
		uint16_t transparency_ = 0; /* Transparent background color */
		uint8_t x_aspect_ratio_ = 0; /* Horizontal pixel size */
		uint8_t y_aspect_ratio_ = 0; /* Vertical pixel size */
		uint16_t page_width_ = 0;    /* Horizontal resolution of display device */
		uint16_t page_height_ = 0;   /* Vertical resolution of display device */
	public:
		BMHD();
		BMHD(bytestream& stream);
	};


	typedef struct color {
		uint8_t r; uint8_t g; uint8_t b;
	} color;


	class CMAP : public CHUNK {
		vector<color> palette_;

	public:
		CMAP();
		CMAP(bytestream& stream);
		const vector<color> GetPalette() const;
	};


	// Optional
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

	// Actual bitmap data
	class BODY : public CHUNK {
		vector<uint8_t> raw_data_;
		// Struct to hold lines in vector

	public:
		BODY();
		BODY(bytestream& stream);
		// Put decoder / exposer here too.
	};

	class UNKNOWN_CHUNK : public CHUNK {
	public:
		UNKNOWN_CHUNK(bytestream& stream);
	};

	enum class CHUNK_T { BMHD, CMAP, CAMG, DPI, BODY };


	class ILBM : public FORM_CONTENTS {
	public:
		map<CHUNK_T, unique_ptr<CHUNK>> chunks_;

		const map <string, CHUNK_T> supported_chunks_ = {
			{ "BMHD", CHUNK_T::BMHD },
			{ "CMAP", CHUNK_T::CMAP },
			{ "CAMG", CHUNK_T::CAMG },
			{ "DPI ", CHUNK_T::DPI },
			{ "BODY", CHUNK_T::BODY }
		};

		// Takes the stream, constructs supported ILBM chunks from it.
		void ChunkFactory(bytestream& stream);
		unique_ptr<CHUNK> ChunkFactoryInternals(bytestream& stream, const CHUNK_T found_chunk) const;

		// Add capability to use both cmap and body to get full image.	

	public:
		ILBM(bytestream& stream);
		const vector<color> GetPalette() const;
	};

	class INVALID_FORM : public FORM_CONTENTS {
	public:
		INVALID_FORM(bytestream&);
	};



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
	In the inverse routine Packer, it's best to encode a 2 byte repeat run as a replicate run except when preceded and followed by a literal run, in which case it's best to merge the three into one literal run. Always encode 3 byte repeats as replicate runs.

	Remember that each row of each scan line of a raster is separately packed.

	[Some versions of Adobe Photoshop incorrectly use the n=128 no-op as a repeat code, which breaks
	strictly conforming readers. To read Photoshop ILBMs, allow the use of n=128 as a repeat. This
	is pretty safe, since no known program writes real no-ops into their ILBMs. The reason n=128 is
	a no-op is historical: the Mac Packbits buffer was only 128 bytes, and a repeat code of 128
	generates 129 bytes.]
	*/


	class FORM {
	private:
		unique_ptr<FORM_CONTENTS> form_contents_;
		uint32_t size_{ 0 };
		string tag_;

		unique_ptr<FORM_CONTENTS> FormFactory(bytestream& stream);

	public:
		FORM(bytestream& stream);

	};
}