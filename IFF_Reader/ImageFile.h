#pragma once

#include "FileData.h"
#include <filesystem>

using std::move;
using std::ofstream;
using std::unique_ptr;
namespace fs = std::filesystem;

// Handler class. Needs a better name.
class ImageFile {
  fs::path filepath;
  unique_ptr<IFFReader::File> file;
  shared_ptr<IFFReader::ILBM> ilbm;
  bool loaded;

public:
  ImageFile() : loaded(false) {}
  ImageFile(const fs::path &path);

  // User friendly error message.
  const string ErrorMessage(const IFFReader::File &f) const;

  // Returns actual ILBM. Possible refactor candidate (to avoid passing ptrs).
  shared_ptr<IFFReader::ILBM> Get() const;

  // Returns the image path.
  const string Path() const;

  // Returns whether this could be OCS corrected.
  const bool OffersOCSColourCorrection() const;

  // Returns whether this is currently OCS corrected.
  const bool UsingOCSColourCorrection() const;

  // Toggles whether or not we are currently correcting OCS values.
  const void ApplyOCSColourCorrection(const bool apply);

  // Whether or not file is done loading.
  const bool IsLoaded() const;
};
