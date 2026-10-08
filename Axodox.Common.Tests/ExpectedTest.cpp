#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Common::Tests
{
  TEST_CLASS(ExpectedTests)
  {
    enum class error_code { too_late };

  public:
    TEST_METHOD(TestValueIsCarriedAndReadable)
    {
      expected<string> value{ string("payload") };

      Assert::IsTrue(bool(value));
      Assert::IsTrue(value.has_value());
      Assert::AreEqual<string>("payload", value.result());
      Assert::AreEqual<string>("payload", *value);
      Assert::AreEqual(size_t{ 7 }, value->size());
    }

    TEST_METHOD(TestErrorCarriesReasonAndHasNoValue)
    {
      expected<string> value = failure("it broke");

      Assert::IsFalse(bool(value));
      Assert::IsFalse(value.has_value());
      Assert::AreEqual<string>("it broke", value.error());
    }

    TEST_METHOD(TestThereIsNoDefaultState)
    {
      //An expected always holds either a result or an error, so there is nothing to default construct.
      static_assert(!is_default_constructible_v<expected<string>>);

      //The void specialization has no result to carry, so a default is its success state.
      static_assert(is_default_constructible_v<expected<>>);
    }

    TEST_METHOD(TestAccessingTheWrongSideThrows)
    {
      expected<string> value{ string("payload") };
      Assert::ExpectException<logic_error>([&] { value.error(); });

      expected<string> failed = failure("it broke");
      Assert::ExpectException<logic_error>([&] { failed.result(); });

      expected<> empty;
      Assert::ExpectException<logic_error>([&] { empty.error(); });
    }

    TEST_METHOD(TestVoidSpecializationSucceedsByDefault)
    {
      expected<> value;

      Assert::IsTrue(bool(value));
      Assert::IsTrue(value.has_value());
    }

    TEST_METHOD(TestVoidSpecializationCarriesError)
    {
      expected<> value = failure("nothing to do");

      Assert::IsFalse(bool(value));
      Assert::AreEqual<string>("nothing to do", value.error());
    }

    TEST_METHOD(TestMoveOnlyValueIsAccepted)
    {
      expected<unique_ptr<int>> value{ make_unique<int>(42) };

      Assert::IsTrue(bool(value));
      Assert::AreEqual(42, **value);
    }

    TEST_METHOD(TestCheckPassesOnSuccess)
    {
      expected<string> value{ string("payload") };
      value.check();

      expected<> empty;
      empty.check();
    }

    TEST_METHOD(TestCheckThrowsTheError)
    {
      expected<string> value = failure("it broke");

      try
      {
        value.check();
        Assert::Fail(L"check() was expected to throw.");
      }
      catch (const runtime_error& exception)
      {
        Assert::AreEqual<string>("it broke", exception.what());
      }
    }

    TEST_METHOD(TestVoidSpecializationCheckThrowsTheError)
    {
      expected<> value = failure("nothing to do");

      Assert::ExpectException<runtime_error>([&] { value.check(); });
    }

    TEST_METHOD(TestValueOrReturnsValueWhenPresent)
    {
      expected<string> value{ string("payload") };

      Assert::AreEqual<string>("payload", value.value_or("fallback"));
    }

    TEST_METHOD(TestValueOrReturnsFallbackOnError)
    {
      expected<string> value = failure("it broke");

      Assert::AreEqual<string>("fallback", value.value_or("fallback"));
    }

    TEST_METHOD(TestCustomErrorTypeIsCarried)
    {
      expected<int, error_code> value = failure(error_code::too_late);

      Assert::IsFalse(bool(value));
      Assert::IsTrue(error_code::too_late == value.error());
      Assert::AreEqual(-1, value.value_or(-1));
    }

    TEST_METHOD(TestFailureIsAnAliasOfUnexpected)
    {
      //failure is the spelling to reach for where the bare unexpected would be ambiguous.
      static_assert(is_same_v<failure<string>, Infrastructure::unexpected<string>>);
      static_assert(is_same_v<decltype(failure("boom")), Infrastructure::unexpected<string>>);

      expected<string> value = failure("boom");
      Assert::AreEqual<string>("boom", value.error());
    }

    TEST_METHOD(TestUnexpectedExposesErrorByAccessor)
    {
      Infrastructure::unexpected<string> error{ string("it broke") };

      Assert::AreEqual<string>("it broke", error.error());
      static_assert(is_same_v<decltype(declval<const Infrastructure::unexpected<string>&>().error()), const string&>);
    }

    TEST_METHOD(TestUnexpectedMovesItsErrorIntoTheExpected)
    {
      //An rvalue unexpected hands its error over rather than copying it.
      Infrastructure::unexpected<string> error{ string("a reason long enough to defeat the small string buffer") };
      auto address = error.error().data();

      expected<int> value = std::move(error);

      Assert::IsTrue(address == value.error().data(), L"the error should have been moved, not copied");
    }

    TEST_METHOD(TestResultsCompareByValue)
    {
      expected<string> first{ string("payload") };
      expected<string> second{ string("payload") };
      expected<string> other{ string("different") };

      Assert::IsTrue(first == second);
      Assert::IsFalse(first == other);
      Assert::IsTrue(first != other);
    }

    TEST_METHOD(TestErrorsCompareByReason)
    {
      expected<string> first = failure("it broke");
      expected<string> second = failure("it broke");
      expected<string> other = failure("it broke differently");

      Assert::IsTrue(first == second);
      Assert::IsFalse(first == other);
    }

    TEST_METHOD(TestResultNeverMatchesError)
    {
      expected<string> value{ string("payload") };
      expected<string> error = failure("it broke");

      Assert::IsFalse(value == error);
      Assert::IsTrue(value != error);
    }

    TEST_METHOD(TestComparesAgainstBareValueAndError)
    {
      expected<string> value{ string("payload") };
      expected<string> error = failure("it broke");

      //A result compares against the value it holds, an error against the reason it carries.
      Assert::IsTrue(value == string("payload"));
      Assert::IsFalse(value == string("different"));
      Assert::IsTrue(error == failure("it broke"));
      Assert::IsFalse(error == failure("something else"));

      //Neither side matches the other kind.
      Assert::IsFalse(value == failure("payload"));
      Assert::IsFalse(error == string("it broke"));
    }

    TEST_METHOD(TestVoidSpecializationCompares)
    {
      expected<> success;
      expected<> alsoSuccess;
      expected<> failed = failure("nothing to do");
      expected<> alsoFailed = failure("nothing to do");

      Assert::IsTrue(success == alsoSuccess);
      Assert::IsTrue(failed == alsoFailed);
      Assert::IsFalse(success == failed);
      Assert::IsTrue(failed == failure("nothing to do"));
    }

    TEST_METHOD(TestUnexpectedCompares)
    {
      Assert::IsTrue(failure("it broke") == failure("it broke"));
      Assert::IsFalse(failure("it broke") == failure("something else"));
    }

    TEST_METHOD(TestFormatUnexpectedBuildsTheMessage)
    {
      expected<int> value = Infrastructure::format_unexpected("property '{}' failed after {} tries", "always", 3);

      Assert::IsFalse(bool(value));
      Assert::AreEqual<string>("property 'always' failed after 3 tries", value.error());
    }

    TEST_METHOD(TestFormatUnexpectedTakesNoArguments)
    {
      expected<> value = Infrastructure::format_unexpected("nothing to do");

      Assert::AreEqual<string>("nothing to do", value.error());
    }

    TEST_METHOD(TestStateIsFixedAtConstruction)
    {
      //The result and the error are private, so an expected cannot change verdict after it is built.
      static_assert(is_same_v<decltype(declval<expected<string>>().result()), const string&>);
      static_assert(is_same_v<decltype(declval<expected<string>>().error()), const string&>);
      static_assert(is_same_v<decltype(declval<expected<>>().error()), const string&>);

      expected<string> value{ string("payload") };
      Assert::IsTrue(value.has_value());
    }

    TEST_METHOD(TestCustomErrorTypeIsFormattedOnCheck)
    {
      expected<void, int> value = failure(7);

      try
      {
        value.check();
        Assert::Fail(L"check() was expected to throw.");
      }
      catch (const runtime_error& exception)
      {
        Assert::AreEqual<string>("7", exception.what());
      }
    }
  };
}
