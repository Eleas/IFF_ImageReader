#pragma once
#include "InterleavedBitmap.h"


namespace IFFReader {
	// List of recognized IFF formats.
	enum class IFF_T { ILBM, UNKNOWN_FORMAT, UNREADABLE, FORM_NOT_FOUND };


	class File {
	private:
		string path_;
		IFF_T type_;
		bytestream stream_;
		uint32_t size_;
		shared_ptr<ILBM> asILBM_;

	public:
		File(const string& path);

		// Returns an ILBM object for displaying or manipulating (empty if 
		// file is invalid).
		shared_ptr<ILBM> AsILBM() const;

		// Returns the type of IFF file that was successfully parsed, if any.
		const IFF_T GetType() const;
	};
}
