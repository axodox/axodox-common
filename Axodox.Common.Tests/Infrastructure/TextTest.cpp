#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.Storage.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  vector<uint8_t> bytes_of(string_view text)
  {
    return vector<uint8_t>(text.begin(), text.end());
  }

  named_enum(traffic_light, red, amber, green);

  named_enum(signal_level, low = 2, medium, high = 0x10, maximum = ~0);

  named_flags(access_rights, none = 0, read = 1, write = 2, execute = 4, read_write = read | write, all = ~0);
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(TextTests)
  {
  public:
    TEST_METHOD(TestNamedEnumFormatting)
    {
      Assert::AreEqual<string>("amber", format("{}", traffic_light::amber));
      Assert::AreEqual<wstring>(L"green", format(L"{}", traffic_light::green));

      // Format specifiers apply to the name, as they would to a string.
      Assert::AreEqual<string>("[  red]", format("[{:>5}]", traffic_light::red));
      Assert::AreEqual<wstring>(L"[red  ]", format(L"[{:<5}]", traffic_light::red));

      // A value without a name falls back to its number.
      Assert::AreEqual<string>("7", format("{}", traffic_light(7)));
    }

    TEST_METHOD(TestNamedEnumExplicitValues)
    {
      // Declared values are used, and an entry without one follows the previous entry.
      Assert::AreEqual(2, int(signal_level::low));
      Assert::AreEqual<string>("low", to_string(signal_level::low));
      Assert::AreEqual<string>("medium", to_string(signal_level(3)));
      Assert::AreEqual<string>("high", to_string(signal_level(16)));
      Assert::AreEqual<string>("maximum", to_string(signal_level(-1)));

      Assert::IsTrue(try_parse<signal_level>("Medium") == signal_level::medium);
      Assert::IsTrue(try_parse<signal_level>("high") == signal_level::high);
      Assert::IsTrue(try_parse<signal_level>("16") == signal_level::high);

      // The values of the declaration do not become names.
      Assert::AreEqual(size_t(4), enum_values<signal_level>().size());
    }

    TEST_METHOD(TestNamedFlagsFormatting)
    {
      // Values with a name of their own use it, including combinations declared in the enum.
      Assert::AreEqual<string>("none", to_string(access_rights::none));
      Assert::AreEqual<string>("read_write", to_string(access_rights::read_write));
      Assert::AreEqual<string>("all", to_string(access_rights::all));

      // Other combinations are written as their single bits.
      Assert::AreEqual<string>("read | execute", to_string(access_rights(5)));

      // Bits without a name fall back to the number.
      Assert::AreEqual<string>("9", to_string(access_rights(9)));
    }

    TEST_METHOD(TestNamedFlagsParsing)
    {
      Assert::IsTrue(try_parse<access_rights>("write") == access_rights::write);
      Assert::IsTrue(try_parse<access_rights>("read | execute") == access_rights(5));
      Assert::IsTrue(try_parse<access_rights>("Read|Write") == access_rights::read_write);
      Assert::IsTrue(try_parse<access_rights>("read | 4") == access_rights(5));
      Assert::IsFalse(try_parse<access_rights>("read | unknown").has_value(), L"unknown flag name parsed");
    }

    TEST_METHOD(TestTryParseTellsInvalidTextApartFromAllBits)
    {
      // A flag with all bits set has the value parse used for invalid text, try_parse keeps them apart.
      Assert::IsTrue(try_parse<access_rights>("all") == access_rights::all);
      Assert::IsTrue(try_parse<access_rights>(" ALL ") == access_rights::all);
      Assert::IsFalse(try_parse<access_rights>("").has_value(), L"empty text parsed");
      Assert::IsFalse(try_parse<access_rights>("bogus").has_value(), L"unknown name parsed");
      Assert::IsFalse(try_parse<access_rights>("4x").has_value(), L"number with trailing text parsed");

      Assert::IsTrue(try_parse<int>("42") == 42);
      Assert::IsFalse(try_parse<int>("42 apples").has_value(), L"number with trailing text parsed");
      Assert::IsFalse(try_parse<int>("").has_value(), L"empty text parsed");
    }

    TEST_METHOD(TestUuidFormatting)
    {
      auto id = uuid::from_string("01234567-89ab-cdef-0123-456789abcdef");
      Assert::IsTrue(id.has_value(), L"uuid did not parse");

      auto text = id->to_string();
      Assert::AreEqual<string>(text, format("{}", *id));
      Assert::AreEqual<wstring>(wstring(text.begin(), text.end()), format(L"{}", *id));
    }

    TEST_METHOD(TestUuidEquality)
    {
      uuid a{ "01234567-89ab-cdef-0123-456789abcdef" };
      uuid b{ "01234567-89ab-cdef-0123-456789abcdef" };
      uuid c{ "01234567-89ab-cdef-0123-456789abcdee" };

      Assert::IsTrue(a == b, L"equal uuids compared unequal");
      Assert::IsTrue(a != c, L"different uuids compared equal");
      Assert::IsTrue(uuid{} == uuid{}, L"empty uuids compared unequal");
    }

    TEST_METHOD(TestTryReadText)
    {
      auto path = filesystem::temp_directory_path() / "axodox_try_read_text.txt";
      auto text = string("first line\nsecond line");

      Axodox::Storage::write_file(path, bytes_of(text));
      Assert::AreEqual(text, Axodox::Storage::try_read_text(path).value_or("<none>"));

      auto withBom = bytes_of(text);
      withBom.insert(withBom.begin(), { 0xEF, 0xBB, 0xBF });
      Axodox::Storage::write_file(path, withBom);
      Assert::AreEqual(text, Axodox::Storage::try_read_text(path).value_or("<none>"));

      filesystem::remove(path);
      Assert::IsFalse(Axodox::Storage::try_read_text(path).has_value(), L"a missing file was read");
    }

    TEST_METHOD(TestTrim)
    {
      Assert::AreEqual<string>("a b", string(trim("  a b \t\r\n")));
      Assert::AreEqual<string>("", string(trim(" \t ")));
      Assert::AreEqual<string>("", string(trim("")));
      Assert::AreEqual<wstring>(L"a b", wstring(trim(L" a b ")));
    }

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
