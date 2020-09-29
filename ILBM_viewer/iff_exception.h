#pragma once
#include <stdexcept>

using std::logic_error;
using std::runtime_error;
using std::string;

class iff_bad_or_mangled_error : public runtime_error {
public:
	explicit iff_bad_or_mangled_error(const string& what_arg) : 
		runtime_error(what_arg) {}
};

class iff_subtype_unimplemented_error : public logic_error {
public:
	explicit iff_subtype_unimplemented_error() : 
		logic_error("This IFF feature is not yet implemented.") {}
};

class iff_headless_error : public iff_bad_or_mangled_error {
public:
	explicit iff_headless_error() : 
		iff_bad_or_mangled_error("IFF appears to be missing its header chunk, and cannot be decoded.") {}
};

class iff_bodiless_error : public iff_bad_or_mangled_error {
public:
	explicit iff_bodiless_error() : 
		iff_bad_or_mangled_error("IFF appears to be missing its body chunk, and cannot be decoded.") {}
};

class iff_reading_error : public iff_bad_or_mangled_error {
public:
	explicit iff_reading_error() : 
		iff_bad_or_mangled_error("Error occurred during reading of IFF chunks. File might be corrupt or nonstandard.") {}
};