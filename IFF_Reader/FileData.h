#pragma once
#include "InterleavedBitmap.h"

namespace IFFReader { // List of recognized IFF formats.
enum class IFF_T { ILBM, UNKNOWN_FORMAT };
enum class IFF_ERRCODE {
  NO_ERROR,
  FILE_NOT_FOUND,
  COULD_NOT_PARSE_AS_IFF,
  COULD_NOT_PARSE_HEAD,
  COULD_NOT_PARSE_BODY
};

class File {
private:
  string path_;
  IFF_T type_;
  IFF_ERRCODE error_code_;
  bytestream stream_;
  uint32_t size_;
  shared_ptr<ILBM> asILBM_;

public:
  File(const string &path);

  // Returns ILBM object to display or manipulate (empty if invalid).
  shared_ptr<ILBM> AsILBM() const;

  // Returns type of IFF file that was successfully parsed, if any.
  const IFF_T GetType() const;

  // Returns specifics of error.
  const IFF_ERRCODE GetError() const;
};
} // namespace IFFReader
