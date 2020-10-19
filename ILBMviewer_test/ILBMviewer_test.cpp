#include "pch.h"
#include "CppUnitTest.h"
#include "Chunk.h"
#include "CommodoreAmiga.h"
#include "Body.h"
#include "ColorLookup.h"
#include "BitmapHeader.h"
#include "FileData.h"
#include "InterleavedBitmap.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ILBMviewertest
{
	TEST_CLASS(ILBMviewertest)
	{
	public:
		
		TEST_METHOD(TestILBMLoads_1)
		{
			IFFReader::File f("../../ILBM_viewer/test files/00A.iff");
			Assert::IsNotNull(f.AsILBM().get());
		}

		// Output the pixel data to a file -- all the color values.
		// Write a test that gets the pixel data and compares those to the file.

	};
}
