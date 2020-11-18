#pragma once

#include "ImageFile.h"
#include "olcPixelGameEngine.h"

using std::cout;
using std::make_shared;
using std::move;
using std::ofstream;
using std::vector;
namespace fs = std::filesystem;

// Renderer class has started to become God object. Should
// be subordinate to actual viewer via composition.
class Renderer : public olc::PixelGameEngine {
  vector<ImageFile> images_;
  size_t current_image = 0;
  double cyclic = 0;
  bool break_requested = false;
  bool break_no_valid_iff = false;
  bool done_loading_files = false;

  void AddImage(ImageFile &img);
  const bool BackKeyReleased();
  const bool ForwardKeyReleased();
  bool OnUserUpdate(float fElapsedTime) override;

public:
  Renderer();
  const vector<uint32_t> GetData(const size_t n) const;
  const size_t GetFilePosByAbspath(const fs::path path) const;
  void DisplayImage();

  // Thread communication.
  const bool RequestedBreak() const;  // Renderer requests file reader to halt.
  void BreakNoValidIFF(); // File reader requests break due to invalid iff.
  const bool InvalidIFF() const; // Test whether file was invalid.
  void DoneLoadingFiles(); // Declare that file reader is done.

  // Called once at the start, so create things here
  bool OnUserCreate() override;
  const bool Viewable() const;
  const bool AddImage(const fs::path &path);
};
