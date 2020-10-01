#pragma once
#include "Chunk.h"
#include "ColorMap.h"

namespace IFFReader {

	// Amiga-specific image data chunk.
	class CAMG : public CHUNK {
		OCSmodes contents_{ 0 };

	public:
		CAMG();
		CAMG(bytestream& stream);

		// This describes Amiga specific modes, such as HAM, EHB etc.
		// It does not describe later-era modes (AGA, HAM-8...)
		const OCSmodes GetModes() const;
	};
}
