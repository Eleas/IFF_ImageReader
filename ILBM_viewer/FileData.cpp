#include "FileData.h"
#include "utility.h"


// Parses IFF file.
IFFReader::File::File(const string& path) : 
	path_(path), size_(0), type_(IFFReader::IFF_T::FORM_NOT_FOUND)
{
	stream_.open(path, std::ios::binary);
	
	if (!stream_.is_open()) {
		return;
	}

	try {
		if (read_tag(stream_) != "FORM") {
			type_ = IFFReader::IFF_T::UNREADABLE;
			return;
		}

		size_ = read_long(stream_);
		const string tag = read_tag(stream_);

		// Expand into a proper factory once we get multiple supported file types.
		if (tag == "ILBM") {
			asILBM_ = shared_ptr<ILBM>(new ILBM(stream_));
			type_ = IFFReader::IFF_T::ILBM;
		}
	}
	catch (...) {  // This badly needs a refactor.
		type_ = IFFReader::IFF_T::UNREADABLE; // Abort if file malformed or missing.
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
