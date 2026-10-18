#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Common::Tests
{
  TEST_CLASS(ExceptionsTests)
  {
  public:
    TEST_METHOD(TestMessageOfStandardException)
    {
      try
      {
        throw runtime_error("standard failure");
      }
      catch (...)
      {
        Assert::AreEqual(string("standard failure"), get_exception_message());
      }
    }

    TEST_METHOD(TestMessageOfWinRtException)
    {
      try
      {
        throw winrt::hresult_error(E_ACCESSDENIED, L"winrt failure");
      }
      catch (...)
      {
        Assert::AreEqual(string("winrt failure"), get_exception_message());
      }
    }

    TEST_METHOD(TestMessageOfUnknownException)
    {
      try
      {
        throw 42;
      }
      catch (...)
      {
        Assert::AreEqual(string("Unknown exception."), get_exception_message());
      }
    }

    TEST_METHOD(TestMessageOfStoredException)
    {
      auto exception = make_exception_ptr(logic_error("stored failure"));
      Assert::AreEqual(string("stored failure"), get_exception_message(exception));
      Assert::AreEqual(string("No exception."), get_exception_message(nullptr));
    }
  };
}
