#include "Body.h"


IFFReader::BODY::BODY()
{
}


// BODY data fetched from stream as raw bytes.
IFFReader::BODY::BODY(bytestream& stream) : CHUNK(stream)
{
	for (uint32_t i = 0; i < GetSize(); ++i) {
		raw_data_.emplace_back(read_byte(stream));
	}
}


const bytefield& IFFReader::BODY::GetRawData() const
{
	return raw_data_;
}


// Unpacks raw data using ByteRun1 encoding. 
//[http://amigadev.elowar.com/read/ADCD_2.1/Devices_Manual_guide/node01C0.html]
const bytefield IFFReader::BODY::GetUnpacked_ByteRun1() const
{
	// We read from the raw data stream.
	const size_t original_size{ raw_data_.size() };

	bytefield unpacked_data; // Destination.
	size_t position { 0 };
	int8_t value { 0 };

	while (position < (original_size - 1)) 
	{   // Make signed.
		value = static_cast<int8_t>(raw_data_.at(position++)); 

		// Do one of two things, n+1 number of time, where n is absolute value.
		for (int i = 0; i < abs(value) + 1; ++i) 
		{   // Copy byte after the instruction (+prevent vector out of range).
			unpacked_data.emplace_back(position < original_size ? 
				raw_data_.at(position) :
				0);

			if (value >= 0) 
			{	// positive: copy that many bytes + 1 straight from original. 
				// negative: copy byte equal number of times (ignoring minus).
				++position;		
			}
		}

		if (value < 0) {
			++position;
		}
	}

	/*[	
		Some versions of Adobe Photoshop incorrectly use the n = 128 no - op as
		a repeat code, which breaks strictly conforming readers. To read Photoshop
		ILBMs, allow the use of n = 128 as a repeat.

		This is pretty safe, since no known program writes real no - ops into their
		ILBMs.

		The reason n = 128 is a no - op is historical : the Mac Packbits buffer was 
		only 128 bytes, and a repeat code of 128 generates 129 bytes.
	]*/

	return move(unpacked_data);
}
