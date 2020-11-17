#include "ImageFile.h"

#include <iostream>

using std::make_shared;
using std::cout;

const string ImageFile::ErrorMessage(const IFFReader::File& f) const
{
	using IFFReader::IFF_ERRCODE;

	switch (file->GetError()) {
	case IFF_ERRCODE::FILE_NOT_FOUND:
		return "No valid IFF file found. Did you check the file path?\n";
	case IFF_ERRCODE::COULD_NOT_PARSE_AS_IFF:
		return "IFF file mangled; cannot open.\n";
	case IFF_ERRCODE::COULD_NOT_PARSE_HEAD:
		return "IFF file mangled; failed to parse head.\n";
	case IFF_ERRCODE::COULD_NOT_PARSE_BODY:
		return "IFF file mangled; failed to parse body.\n";
	default:
		if (file->GetType() == IFFReader::IFF_T::UNKNOWN_FORMAT) {
			return "This ILBM has an as-yet unsupported format.\n";
		}
	}
	return string();
}


ImageFile::ImageFile(const fs::path& path) : filepath(path), loaded(false)
{
	file = unique_ptr<IFFReader::File>
		(new IFFReader::File(path.string()));

	if (const string error_text = ErrorMessage(*file.get());
		!error_text.empty())
	{
		cout << error_text;
		return;
	}

	ilbm = make_shared<IFFReader::ILBM>(*file->AsILBM());
	loaded = true;
}


shared_ptr<IFFReader::ILBM> ImageFile::Get() const
{
	return ilbm;
}


const bool ImageFile::IsPath(const fs::path path) const
{
	return path == filepath;
}


const bool ImageFile::OffersOCSColourCorrection() const
{
	return ilbm->allows_ocs_correction();
}


const bool ImageFile::UsingOCSColourCorrection() const {
	return ilbm->using_ocs_correction();
}


const void ImageFile::ApplyOCSColourCorrection(const bool apply)
{
	ilbm->color_correction(apply);
}

const bool ImageFile::IsLoaded() const
{
	return loaded;
}
