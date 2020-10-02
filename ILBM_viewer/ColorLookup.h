#pragma once
#include <cstdint>
#include <vector>

using std::vector;

namespace IFFReader {
	class ColorLookup
	{
		vector<uint32_t> colors_;

	public:
		ColorLookup();
		ColorLookup(const vector<uint32_t>& colors);

		// Adds EHB colors.
		void TreatAsEHB();

		const uint32_t at(const unsigned int position) const;
	};
}
