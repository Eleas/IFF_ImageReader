#include "CommodoreAmiga.h"


IFFReader::CAMG::CAMG()
{
}


IFFReader::CAMG::CAMG(bytestream& stream) : 
	CHUNK(stream)
{	// Parse various screen modes.
	contents_ = IFFReader::OCSmodes(IFFReader::read_long(stream)); 
}


/*
There are several CAMG values saved by old ILBM writers which are invalid
modeID values.  An ILBM writer will produce ILBMs with invalid modeID
values if it:
	*  fails to mask out SPRITES|VP_HIDE|GENLOCK_AUDIO|GENLOCK_VIDEO.
	*  saves a 2.0 extended view mode as 16 bits rather than saving the
	   32-bit modeID returned by GetVPModeID().
	*  saves garbage in the upper word of the CAMG value.
All valid modeIDs either have an upper word of 0 and do not have the
<graphics/view.h> EXTENDED_MODE bit set in the low word, or have a non-0
upper word and do have the EXTENDED_MODE bit set in the lower word. CAMG
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


IFFReader::OCSmodes::OCSmodes(const uint32_t contents) : 
	contents(contents) 
{
	Interlace = (contents & LACE);
	DoubleScan = (contents & DOUBLESCAN);
	ExtraHalfBrite = (contents & EXTRA_HALFBRITE);
	HoldAndModify = (contents & HAM);
	Hires = (contents & HIRES);
	SuperHires = (contents & SUPERHIRES);
	DualPlayfield = (contents & DUALPF);
	DualPlayfieldBAPriority = (contents & PFBA);
}


// Express screen modes that only require CAMG to evaluate.
const IFFReader::OCSmodes IFFReader::CAMG::GetModes() const
{
	return contents_;
}
