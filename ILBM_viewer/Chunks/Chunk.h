#pragma once
#include "utility.h"

namespace IFFReader {

	// Common chunk behavior.
	class CHUNK {
		uint32_t size_;

	public:
		CHUNK();
		CHUNK(bytestream& stream);
		virtual ~CHUNK();

		// Size of data contained by chunk, in bytes.
		virtual const uint32_t GetSize() const;
	};
}
