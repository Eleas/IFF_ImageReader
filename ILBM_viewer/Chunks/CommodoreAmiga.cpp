#include "CommodoreAmiga.h"


IFFReader::CAMG::CAMG()
{
}


IFFReader::CAMG::CAMG(bytestream& stream) : CHUNK(stream)
{
	contents_ = IFFReader::OCSmodes(IFFReader::read_long(stream)); // Parse various screen modes.
}



// Express screen modes that only require CAMG to evaluate.
const IFFReader::OCSmodes IFFReader::CAMG::GetModes() const
{
	return contents_;
}
