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

		virtual const uint32_t GetSize() const;

		// Stores tag as as a string. Only implemented for UNKNOWN chunk.
		virtual void AddTagLiteral(const string tag);
	};
}
