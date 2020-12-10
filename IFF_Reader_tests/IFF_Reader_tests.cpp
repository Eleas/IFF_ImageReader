#include "BitmapHeader.h"
#include "Body.h"
#include "Chunk.h"
#include "ColorLookup.h"
#include "CommodoreAmiga.h"
#include "CppUnitTest.h"
#include "FileData.h"
#include "InterleavedBitmap.h"
#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Test files kept in "../IFF_Reader/test files"
namespace ILBMviewertest {
const bool compare(const string name) {
  const string preamble = "../../IFF_Reader";
  const string first = preamble + "/test files/" + name + ".iff";
  const string second = preamble + "_tests/test dumps/" + name + ".tst";

  IFFReader::File f(first);
  const auto data = f.AsILBM();

  std::ifstream f2(second, std::ios::binary);
  Assert::IsTrue(f2.is_open() == true);
  if (!f2.is_open()) {
    return false;
  }

  std::vector<uint32_t> testfile_contents;
  while (f2.good()) {
    uint32_t buffer;
    f2.read(reinterpret_cast<char *>(&buffer), 4);
    testfile_contents.push_back(buffer);
  }

  size_t o = 0;
  for (unsigned int y = 0; y < data->height(); ++y) {
    for (unsigned int x = 0; x < data->width(); ++x) {
      auto px = data->color_at(x, y);
      if (testfile_contents.at(o++) != px) {
        f2.close();
        return false;
      }
    }
  }
  f2.close();
  return true;
}

TEST_CLASS(ILBMviewertest){
  public :

      TEST_METHOD(TestILBMLoads_1){
          IFFReader::File f("../../IFF_Reader/test files/00A.iff");
Assert::IsNotNull(f.AsILBM().get());
} // namespace ILBMviewertest

// Try all file data against existing tests.
TEST_METHOD(TestILBMFileRegression00A) { Assert::IsTrue(compare("00A")); }

TEST_METHOD(TestILBMFileRegression01A) { Assert::IsTrue(compare("01A")); }

TEST_METHOD(TestILBMFileRegression01B) { Assert::IsTrue(compare("01B")); }

TEST_METHOD(TestILBMFileRegression02A) { Assert::IsTrue(compare("02A")); }

TEST_METHOD(TestILBMFileRegression02B) { Assert::IsTrue(compare("02B")); }
}
;
}
