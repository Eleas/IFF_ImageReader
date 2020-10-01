#include "ColorMap.h"
#include "Chunk.h"
#include "utility.h"

IFFReader::CMAP::CMAP()
{
}


// Extracts palette as a collection of colors.
IFFReader::CMAP::CMAP(bytestream& stream) : CHUNK(stream)
{
	palette_.clear();
	const auto color_count = GetSize() / 3;	 // 3 bytes: R,G,B.
	palette_.resize(color_count);

	for (auto& color : palette_) {
		color = {
			read_byte(stream),
			read_byte(stream),
			read_byte(stream)
		};
	}
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

// Various flags for parsing OCS modes
constexpr uint32_t GENLOCK_VIDEO = 0x0002;  //
constexpr uint32_t LACE = 0x0004;  //
constexpr uint32_t DOUBLESCAN = 0x0008;
constexpr uint32_t SUPERHIRES = 0x0020;
constexpr uint32_t PFBA = 0x0040;
constexpr uint32_t EXTRA_HALFBRITE = 0x0080;
constexpr uint32_t GENLOCK_AUDIO = 0x0100;
constexpr uint32_t DUALPF = 0x0400;
constexpr uint32_t HAM = 0x0800;
constexpr uint32_t EXTENDED_MODE = 0x1000;
constexpr uint32_t VP_HIDE = 0x2000;
constexpr uint32_t SPRITES = 0x4000;
constexpr uint32_t HIRES = 0x8000;


IFFReader::OCSmodes::OCSmodes(uint32_t contents) : contents(contents) {
	Interlace = (contents & LACE);
	DoubleScan = (contents & DOUBLESCAN);
	ExtraHalfBrite = (contents & EXTRA_HALFBRITE);
	HoldAndModify = (contents & HAM);
	Hires = (contents & HIRES);
	SuperHires = (contents & SUPERHIRES);
	DualPlayfield = (contents & DUALPF);
	DualPlayfieldBAPriority = (contents & PFBA);
}


const IFFReader::colors IFFReader::CMAP::GetPalette(const OCSmodes& mode) const
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

