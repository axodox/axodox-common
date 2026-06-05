#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  vector<uint8_t> bytes_of(string_view text)
  {
    return vector<uint8_t>(text.begin(), text.end());
  }
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(TextTests)
  {
  public:
    // RFC 4648 test vectors: "" -> "", "f" -> "Zg==", "fo" -> "Zm8=", "foo" -> "Zm9v",
    // "foob" -> "Zm9vYg==", "fooba" -> "Zm9vYmE=", "foobar" -> "Zm9vYmFy".
    TEST_METHOD(TestBase64EncodingRfcVectors)
    {
      Assert::AreEqual<string>("", encode_base64(bytes_of("")));
      Assert::AreEqual<string>("Zg==", encode_base64(bytes_of("f")));
      Assert::AreEqual<string>("Zm8=", encode_base64(bytes_of("fo")));
      Assert::AreEqual<string>("Zm9v", encode_base64(bytes_of("foo")));
      Assert::AreEqual<string>("Zm9vYg==", encode_base64(bytes_of("foob")));
      Assert::AreEqual<string>("Zm9vYmE=", encode_base64(bytes_of("fooba")));
      Assert::AreEqual<string>("Zm9vYmFy", encode_base64(bytes_of("foobar")));
    }

    TEST_METHOD(TestBase64DecodingRfcVectors)
    {
      auto decode = [](string_view text) {
        vector<uint8_t> result;
        Assert::IsTrue(try_decode_base64(text, result), L"valid base64 failed to decode");
        return result;
      };

      Assert::IsTrue(bytes_of("") == decode(""));
      Assert::IsTrue(bytes_of("f") == decode("Zg=="));
      Assert::IsTrue(bytes_of("fo") == decode("Zm8="));
      Assert::IsTrue(bytes_of("foo") == decode("Zm9v"));
      Assert::IsTrue(bytes_of("foob") == decode("Zm9vYg=="));
      Assert::IsTrue(bytes_of("fooba") == decode("Zm9vYmE="));
      Assert::IsTrue(bytes_of("foobar") == decode("Zm9vYmFy"));
    }

    TEST_METHOD(TestBase64RoundTripAllByteValues)
    {
      // Every possible byte value, so all 6-bit groupings and both padding cases are exercised.
      vector<uint8_t> data(256);
      for (size_t i = 0; i < data.size(); ++i) data[i] = uint8_t(i);

      vector<uint8_t> parsed;
      Assert::IsTrue(try_decode_base64(encode_base64(data), parsed), L"round-trip decode failed");
      Assert::IsTrue(data == parsed, L"binary buffer did not survive the base64 round-trip");
    }

    TEST_METHOD(TestBase64RejectsMalformedInput)
    {
      vector<uint8_t> result;
      Assert::IsFalse(try_decode_base64("Zm9v*", result), L"illegal character accepted");
      Assert::IsFalse(try_decode_base64("Zg==Zg==", result), L"data after padding accepted");
      // "Zh==" has non-zero leftover bits in the final sextet.
      Assert::IsFalse(try_decode_base64("Zh==", result), L"non-canonical trailing bits accepted");
    }
  };
}
