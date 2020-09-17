#pragma once
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

using std::basic_ifstream;
using std::make_shared;
using std::make_unique;
using std::map;
using std::move;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;


namespace ILBMReader {
	typedef basic_ifstream<uint8_t> bytestream;
	const string read_tag(bytestream& stream);
	const uint32_t read_long(bytestream& stream);
	const uint16_t read_word(bytestream& stream);
	const uint8_t read_byte(bytestream& stream);


	// Common behavior (size) and polymorphism
	class CHUNK {
		uint32_t size_;

	public:
		CHUNK();
		CHUNK(bytestream& stream);
		virtual ~CHUNK();

		virtual const uint32_t GetSize() const;
		virtual void AddTagLiteral(const string tag);
	};


	// Stub, but useful if we want to expand from ILBM to other stuff
	class FORM_CONTENTS : public CHUNK {
	};


	// Instead of nulling them, set them in initializer.
	class BMHD : public CHUNK {
		uint16_t width_;
		uint16_t height_;
		uint16_t xcoordinate_;
		uint16_t ycoordinate_;
		uint8_t bitplanes_;
		uint8_t masking_;
		uint8_t compression_;
		uint16_t transparency_; /* Transparent background color */
		uint8_t x_aspect_ratio_; /* Horizontal pixel size */
		uint8_t y_aspect_ratio_; /* Vertical pixel size */
		uint16_t page_width_;    /* Horizontal resolution of display device */
		uint16_t page_height_;   /* Vertical resolution of display device */

	public:
		BMHD();
		BMHD(bytestream& stream);

		const uint16_t GetWidth() const;
		const uint16_t GetHeight() const;
		const uint16_t GetBitplanes() const;
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

	public:
		BODY();
		BODY(bytestream& stream);

		// Use if compression bit is unset.
		const vector<uint8_t>& GetRawData() const;		

		// Use if compression bit is set.
		const vector<uint8_t> GetUnpackedData() const;  
	};


	class UNKNOWN : public CHUNK {
		vector<string> unknown_chunk_names_;

	public:
		UNKNOWN();
		UNKNOWN(bytestream& stream);

		// Stores tag strings, creating a log list of unparsed ands skipped tags.
		void AddTagLiteral(const string tag) override;
	};


	enum class CHUNK_T { BMHD, CMAP, CAMG, DPI, BODY, UNKNOWN };


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

		// Constructs supported ILBM chunks from stream.
		void ChunkFactory(bytestream& stream);

		// Fabricates appropriate chunk from stream.
		unique_ptr<CHUNK> ChunkFactoryInternals(bytestream& stream, const CHUNK_T found_chunk) const;

		// To do: add capability to use both cmap and body to get full image.	
		const std::array<uint8_t, 8> GetByteData(uint8_t byte);

	public:
		ILBM(bytestream& stream);

		const BMHD GetHeader() const;
		const vector<color> GetPalette() const;
		const vector<uint8_t> GetBitData() const;
	};


	class INVALID_FORM : public FORM_CONTENTS {
	public:
		INVALID_FORM(bytestream&);
	};


	class FORM : public CHUNK {
	private:
		shared_ptr<ILBM> form_contents_;
		uint32_t size_{ 0 };

	public:
		FORM(bytestream& stream);
		shared_ptr<ILBM> Get_ILBM() const;
	};

	class File {
	private:
		shared_ptr<FORM> file_contents_;

	public:
		File(const string& path);
		const shared_ptr<ILBM> GetAsILBM() const;
	};

}
