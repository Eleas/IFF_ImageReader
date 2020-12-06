#pragma once
#include "Chunk.h"
#include "ColorMap.h"

namespace IFFReader {
struct OCSmodes {
  uint32_t contents;
  bool HoldAndModify = false;
  bool ExtraHalfBrite = false;
  bool GenlockVideo = false;
  bool GenlockAudio = false;
  bool Interlace = false;
  bool Hires = false;      // Cannot be combined with SuperHires
  bool SuperHires = false; // Cannot be combined with Hires
  bool Sprites = false;    // Using sprites, so sprite palettes will be loaded
  bool DualPlayfield = false;
  bool DualPlayfieldBAPriority = false;
  bool ViewportHide = false; // Whether or not to hide the viewport
  bool DoubleScan = false;   // Uses double scan rate

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

// Amiga-specific image data chunk.
class CAMG : public CHUNK {
  OCSmodes contents_{0};

public:
  CAMG();
  CAMG(bytestream &stream);

  // This describes Amiga specific modes, such as HAM, EHB etc.
  // It does not describe later-era modes (AGA, HAM-8...)
  const OCSmodes GetModes() const;
};
} // namespace IFFReader
