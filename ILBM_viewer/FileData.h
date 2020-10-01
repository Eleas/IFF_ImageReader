#pragma once
#include "InterleavedBitmap.h"


namespace IFFReader {
	// List of recognized IFF formats.
	enum class IFF_T { ILBM, UNKNOWN_FORMAT, UNREADABLE, FORM_NOT_FOUND };

	// List of graphical modes.
	enum class GMODES_T { OCS, EHB, HAM6, SHAM, AGA, HAM8, TRUECOLOR };

	class File {
	private:
		string path_;
		IFF_T type_;
		bytestream stream_;
		uint32_t size_;
		shared_ptr<ILBM> asILBM_;

	public:
		File(const string& path);

		shared_ptr<ILBM> AsILBM() const;
		const IFF_T GetType() const;
	};
}
