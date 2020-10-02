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

		vector<uint32_t>& GetColors();
		virtual const uint32_t at(const unsigned int index);
	};

	class ColorLookupEHB : public ColorLookup
	{
	public:
		ColorLookupEHB();
		ColorLookupEHB(const vector<uint32_t>& colors);

		const uint32_t at(const unsigned int index) override;
	};

	class ColorLookupHAM : public ColorLookup
	{
		vector<uint8_t>& data_;
		uint32_t previous_color_;
	public:
		ColorLookupHAM(const vector<uint32_t>& colors, vector<uint8_t>& data);

		const uint32_t at(const unsigned int index) override;
	};

}
