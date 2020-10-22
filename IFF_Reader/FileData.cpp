#include "FileData.h"
#include "utility.h"


// Parses IFF file.
IFFReader::File::File(const string& path) : 
	path_(path), size_(0), type_(IFFReader::IFF_T::UNKNOWN_FORMAT), error_code_(IFFReader::IFF_ERRCODE::FILE_NOT_FOUND)
{
	stream_.open(path, std::ios::binary);

	if (!stream_.is_open()) {
		return; // Automatically returns file not found.
	}

	try {
		if (read_tag(stream_) != "FORM") {
			error_code_ = IFFReader::IFF_ERRCODE::COULD_NOT_PARSE_AS_IFF;
			return;
		}

		size_ = read_long(stream_);
		const string tag = read_tag(stream_);

		// Expand into a proper factory once we get multiple supported file types.
		if (tag == "ILBM") {
			asILBM_ = shared_ptr<ILBM>(new ILBM(stream_));
			type_ = IFFReader::IFF_T::ILBM;
			error_code_ = IFF_ERRCODE::NO_ERROR;
		}
	}
	catch (...) {  // Consider refactoring this part.
		error_code_ = IFFReader::IFF_ERRCODE::COULD_NOT_PARSE_AS_IFF; // Abort if file malformed or missing.
	}
}


// Returns blank pointer if file was not parsed. Have fun with that.
shared_ptr<IFFReader::ILBM> IFFReader::File::AsILBM() const
{
	if (type_ != IFFReader::IFF_T::ILBM) {
		return shared_ptr<IFFReader::ILBM>();
	}
	return asILBM_;
}


// Yields identified subtype of IFF (ILBM, etc) that the reader can currently parse.
const IFFReader::IFF_T IFFReader::File::GetType() const
{
	return type_; 
}


const IFFReader::IFF_ERRCODE IFFReader::File::GetError() const
{
	return error_code_;
}
