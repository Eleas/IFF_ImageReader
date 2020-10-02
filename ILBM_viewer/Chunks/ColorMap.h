#pragma once
#include "utility.h"
#include "Chunk.h"

namespace IFFReader {

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

		OCSmodes(const uint32_t contents);
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

		// Extracts palette from raw data.
		const colors GetPalette(const OCSmodes& mode) const;
	};
}
