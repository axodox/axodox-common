#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  //Counts the live instances, so ownership mistakes show up as leaks or double frees
  struct TrackedValue
  {
    inline static int InstanceCount = 0;

    int Value;

    explicit TrackedValue(int value = 0) :
      Value(value)
    {
      InstanceCount++;
    }

    TrackedValue(const TrackedValue& other) :
      Value(other.Value)
    {
      InstanceCount++;
    }

    virtual ~TrackedValue()
    {
      InstanceCount--;
    }

    virtual int Kind() const
    {
      return 0;
    }

    bool operator==(const TrackedValue& other) const
    {
      return Kind() == other.Kind() && Value == other.Value;
    }
  };

  struct DerivedTrackedValue : public TrackedValue
  {
    using TrackedValue::TrackedValue;

    virtual int Kind() const override
    {
      return 1;
    }
  };

  struct NonCopyableValue
  {
    NonCopyableValue() = default;
    NonCopyableValue(const NonCopyableValue&) = delete;
  };
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(ValuePtrTests)
  {
  public:
    TEST_METHOD_INITIALIZE(Initialize)
    {
      TrackedValue::InstanceCount = 0;
    }

    TEST_METHOD(TestEmpty)
    {
      value_ptr<TrackedValue> value;

      Assert::IsFalse(bool(value));
      Assert::IsTrue(value == nullptr);
      Assert::IsNull(value.get());

      value_ptr<TrackedValue> copy = value;
      Assert::IsTrue(copy == nullptr);
    }

    TEST_METHOD(TestMakeValue)
    {
      {
        auto value = make_value<TrackedValue>(5);

        Assert::IsTrue(bool(value));
        Assert::AreEqual(5, value->Value);
        Assert::AreEqual(5, (*value).Value);
        Assert::AreEqual(1, TrackedValue::InstanceCount);
      }

      Assert::AreEqual(0, TrackedValue::InstanceCount);
    }

    TEST_METHOD(TestCopyIsDeep)
    {
      {
        auto source = make_value<TrackedValue>(5);
        auto copy = source;

        Assert::IsTrue(source.get() != copy.get());
        Assert::IsTrue(source == copy);
        Assert::AreEqual(2, TrackedValue::InstanceCount);

        copy->Value = 6;
        Assert::AreEqual(5, source->Value);
        Assert::IsFalse(source == copy);

        //Assignment replaces the previous value
        copy = source;
        Assert::AreEqual(5, copy->Value);
        Assert::AreEqual(2, TrackedValue::InstanceCount);

        //Self assignment keeps the value
        auto& alias = copy;
        copy = alias;
        Assert::AreEqual(5, copy->Value);
        Assert::AreEqual(2, TrackedValue::InstanceCount);
      }

      Assert::AreEqual(0, TrackedValue::InstanceCount);
    }

    TEST_METHOD(TestCopyThroughBaseKeepsDerivedType)
    {
      {
        value_ptr<TrackedValue> source = make_value<DerivedTrackedValue>(3);
        auto copy = source;

        Assert::AreEqual(1, copy->Kind());
        Assert::AreEqual(3, copy->Value);
        Assert::IsTrue(source == copy);
      }

      Assert::AreEqual(0, TrackedValue::InstanceCount);
    }

    TEST_METHOD(TestMoveTransfersOwnership)
    {
      {
        auto source = make_value<TrackedValue>(5);
        auto pointer = source.get();

        auto target = move(source);
        Assert::IsTrue(source == nullptr);
        Assert::IsTrue(target.get() == pointer);
        Assert::AreEqual(1, TrackedValue::InstanceCount);

        //The moved value still copies, as the copy constructor moved along with it
        auto copy = target;
        Assert::AreEqual(5, copy->Value);
      }

      Assert::AreEqual(0, TrackedValue::InstanceCount);
    }

    TEST_METHOD(TestEquality)
    {
      value_ptr<TrackedValue> empty;
      auto a = make_value<TrackedValue>(1);
      auto b = make_value<TrackedValue>(1);
      auto c = make_value<TrackedValue>(2);

      Assert::IsTrue(empty == value_ptr<TrackedValue>{});
      Assert::IsFalse(empty == a);
      Assert::IsFalse(a == empty);
      Assert::IsTrue(a == b);
      Assert::IsFalse(a == c);
    }

    TEST_METHOD(TestResetAndRelease)
    {
      auto value = make_value<TrackedValue>(5);
      value.reset();
      Assert::IsTrue(value == nullptr);
      Assert::AreEqual(0, TrackedValue::InstanceCount);

      value = make_value<TrackedValue>(6);
      unique_ptr<TrackedValue> released{ value.release() };
      Assert::IsTrue(value == nullptr);
      Assert::AreEqual(6, released->Value);
      Assert::AreEqual(1, TrackedValue::InstanceCount);
    }

    TEST_METHOD(TestUniquePointerConversions)
    {
      {
        value_ptr<TrackedValue> value{ make_unique<DerivedTrackedValue>(4) };
        Assert::AreEqual(1, value->Kind());

        //Converting back copies, the value_ptr keeps its own value
        auto copy = static_cast<unique_ptr<TrackedValue>>(value);
        Assert::IsTrue(copy.get() != value.get());
        Assert::AreEqual(1, copy->Kind());
        Assert::AreEqual(2, TrackedValue::InstanceCount);
      }

      Assert::AreEqual(0, TrackedValue::InstanceCount);
    }

    TEST_METHOD(TestCopyingNonCopyableValueThrows)
    {
      auto value = make_value<NonCopyableValue>();

      Assert::ExpectException<exception>([&] { auto copy = value; });
    }
  };
}
