#pragma once

#include "ImageFile.h"
#include "olcPixelGameEngine.h"

using std::ofstream;
using std::vector;
namespace fs = std::filesystem;

enum class TVStandard { PAL, NTSC };

// Renderer class has started to become God object. Should
// be subordinate to actual viewer via composition.
class Renderer : public olc::PixelGameEngine {
  vector<ImageFile> images_;
  size_t current_image = 0;
  double cyclic = 0;
  bool break_requested = false;
  bool break_no_valid_iff = false;
  bool done_loading_files = false;
  TVStandard tv_standard_ = TVStandard::PAL;

  // Adds given image to the file list.
  void AddImage(ImageFile &img);

  // Tests if user wants to go back one image.
  const bool BackKeyReleased();

  // Tests if user wants to advance one image.
  const bool ForwardKeyReleased();

  // Runs every actual update frame.
  bool OnUserUpdate(float fElapsedTime) override;

  // Used to tell system how often we should update the screen.
  // (Faster updates than this are not a thing on the Amiga).
  const unsigned int FrameDuration() const;

  // Draw the image to screen.
  void DisplayImage();

public:
  Renderer();

  // Returns the stored image file as an array of integers.
  const vector<uint32_t> GetData(const size_t n) const;

  // If file has been loaded by viewer, returns index; otherwise
  // returns total file count. Used by test generation.
  const size_t GetFilePosByAbspath(const fs::path path) const;

  // Thread communication.
  // Renderer requests file reader to halt.
  const bool RequestedBreak() const;

  // Thread communication.
  // File reader requests break due to invalid iff.
  void BreakNoValidIFF();

  // Thread communication.
  // Test whether file was invalid.
  const bool InvalidIFF() const;

  // Thread communication.
  // Declare that file reader is done.
  void DoneLoadingFiles();

  // Called once at the start, so create things here
  bool OnUserCreate() override;

  // Can we display one or more files?
  const bool Viewable() const;

  // Open this file and add it to the viewable files.
  const bool AddImage(const fs::path &path);
};
