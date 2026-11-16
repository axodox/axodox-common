#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  template<typename TException>
  string message_of(TException exception)
  {
    try
    {
      throw exception;
    }
    catch (...)
    {
      return get_error_message();
    }
  }
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(ErrorTests)
  {
  public:
    TEST_METHOD(TestStdExceptionGivesItsWhat)
    {
      Assert::AreEqual<string>("boom", message_of(runtime_error("boom")));
    }

    TEST_METHOD(TestHresultErrorGivesItsMessage)
    {
      Assert::AreEqual<string>("The operation failed.", message_of(winrt::hresult_error(E_FAIL, L"The operation failed.")));
    }

    TEST_METHOD(TestOtherExceptionGivesUnexpectedError)
    {
      Assert::AreEqual<string>("Unexpected error", message_of(42));
    }
  };
}
