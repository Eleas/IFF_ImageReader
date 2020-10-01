#pragma once
#include "Chunk.h"
#include "utility.h"

namespace IFFReader {

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
}
