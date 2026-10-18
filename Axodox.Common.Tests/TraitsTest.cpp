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

  class abstract_service
  {
  public:
    virtual ~abstract_service() = default;
    virtual int number() const = 0;
  };

  class concrete_service : public abstract_service
  {
  public:
    virtual int number() const override { return 42; }
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

    TEST_METHOD(TestSupportsNewExcludesAbstractTypes)
    {
      static_assert(supports_new<custom_value>);
      static_assert(!supports_new<abstract_service>);
    }

    TEST_METHOD(TestSupportsEqualsDetectsComparisonOperators)
    {
      static_assert(supports_equals<int>);
      static_assert(supports_equals<string>);
      static_assert(!supports_equals<custom_value>);

      static_assert(supports_not_equals<string>);
      static_assert(!supports_not_equals<custom_value>);
    }

    TEST_METHOD(TestValuePtrComparesByValueWhenSupported)
    {
      auto a = make_value<string>("text");
      auto b = make_value<string>("text");
      auto c = make_value<string>("other");
      value_ptr<string> empty;

      Assert::IsTrue(a == b);
      Assert::IsTrue(a != c);
      Assert::IsTrue(empty == value_ptr<string>());
      Assert::IsTrue(a != empty);

      static_assert(supports_equals<value_ptr<string>>);
      static_assert(!supports_equals<value_ptr<custom_value>>);
    }

    TEST_METHOD(TestContainerResolvesAnAbstractTypeRegisteredWithAnInstance)
    {
      dependency_container container;
      container.add<abstract_service>(make_shared<concrete_service>());
      Assert::AreEqual(42, container.resolve<abstract_service>()->number());
    }
  };
}
