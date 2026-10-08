#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  struct custom_value
  {
    int number = 7;
    string text = "unset";
  };
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(TraitsTests)
  {
  public:
    TEST_METHOD(TestDefaultValueOfPrimitiveIsZeroInitialized)
    {
      Assert::AreEqual(0, default_value<int>);
      Assert::AreEqual(0.0, default_value<double>);
      Assert::IsFalse(default_value<bool>);
    }

    TEST_METHOD(TestDefaultValueOfClassUsesItsDefaultConstructor)
    {
      Assert::AreEqual(7, default_value<custom_value>.number);
      Assert::AreEqual<string>("unset", default_value<custom_value>.text);
    }

    TEST_METHOD(TestDefaultValueOfContainerIsEmpty)
    {
      Assert::IsTrue(default_value<string>.empty());
      Assert::IsTrue(default_value<vector<int>>.empty());
    }

    TEST_METHOD(TestDefaultValueIsAStableInstance)
    {
      //It is handed out by reference, so every read must name the same object.
      Assert::IsTrue(&default_value<string> == &default_value<string>);
    }

    TEST_METHOD(TestDefaultValueIsConstAndBindsToConstReference)
    {
      const string& text = default_value<string>;
      Assert::IsTrue(text.empty());

      static_assert(is_const_v<remove_reference_t<decltype(default_value<string>)>>);
    }
  };
}
