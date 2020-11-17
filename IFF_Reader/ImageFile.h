#pragma once

#include "FileData.h"
#include <filesystem>

using std::move;
using std::ofstream;
using std::unique_ptr;
namespace fs = std::filesystem;

// Handler class. Needs a better name.
class ImageFile
{
	fs::path filepath;
	unique_ptr<IFFReader::File> file;
	shared_ptr<IFFReader::ILBM> ilbm;
	bool loaded;

public:
	ImageFile() : loaded(false) {}
	ImageFile(const fs::path& path);

	const string ErrorMessage(const IFFReader::File& f) const;
	shared_ptr<IFFReader::ILBM> Get() const;
	const bool IsPath(const fs::path path) const;
	const bool OffersOCSColourCorrection() const;
	const bool UsingOCSColourCorrection() const;
	const void ApplyOCSColourCorrection(const bool apply);
	const bool IsLoaded() const;
};
