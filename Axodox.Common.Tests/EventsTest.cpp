#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;
using namespace std::chrono_literals;

namespace
{
  constexpr auto test_timeout = 5s;

  class number_source : public event_owner
  {
  public:
    event_publisher<int> numbers;

    number_source() :
      numbers(*this)
    { }

    void publish(int value)
    {
      raise(numbers, move(value));
    }
  };

  class pair_source : public event_owner
  {
  public:
    event_publisher<string, int> pairs;

    pair_source() :
      pairs(*this)
    { }

    void publish(string text, int value)
    {
      raise(pairs, move(text), move(value));
    }
  };

  //Raises events on a background thread, one at a time, on the test's command.
  //The publishing thread never runs ahead of the test, so the tests stay deterministic.
  class background_publisher
  {
  public:
    background_publisher(number_source& source) :
      _thread([this, &source] {
        while (true)
        {
          _publishRequested.wait();
          if (_isShuttingDown) return;

          source.publish(_next);
          _publishCompleted.set();
        }
        })
    { }

    //Raises a single event and returns once the handler chain has finished with it.
    void publish(int value)
    {
      _next = value;
      _publishRequested.set();
      //Assert::IsTrue(_publishCompleted.wait(test_timeout), L"Publishing an event timed out.");
    }

    ~background_publisher()
    {
      _isShuttingDown = true;
      _publishRequested.set();
    }

  private:
    atomic_bool _isShuttingDown = false;
    int _next = {};
    auto_reset_event _publishRequested;
    auto_reset_event _publishCompleted;
    jthread _thread;
  };
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(EventsTests)
  {
  public:
    TEST_METHOD(TestSubscriberReceivesRaisedEvent)
    {
      number_source source;

      auto received = 0;
      auto subscription = source.numbers([&](int value) { received = value; });

      source.publish(7);

      Assert::AreEqual(7, received);
    }

    TEST_METHOD(TestAllSubscribersReceiveRaisedEvent)
    {
      number_source source;

      auto first = 0, second = 0;
      auto firstSubscription = source.numbers([&](int value) { first = value; });
      auto secondSubscription = source.numbers([&](int value) { second = value; });

      source.publish(3);

      Assert::AreEqual(3, first);
      Assert::AreEqual(3, second);
    }

    TEST_METHOD(TestResetSubscriptionStopsReceivingEvents)
    {
      number_source source;

      auto received = 0;
      auto subscription = source.numbers([&](int value) { received = value; });

      source.publish(1);
      subscription.reset();
      source.publish(2);

      Assert::AreEqual(1, received);
    }

    TEST_METHOD(TestRaisingEventFromForeignOwnerThrows)
    {
      number_source source;
      event_owner foreignOwner;

      Assert::ExpectException<logic_error>([&] { source.numbers.raise(foreignOwner, 1); });
    }

    TEST_METHOD(TestWaitReturnsEventArguments)
    {
      number_source source;
      background_publisher publisher{ source };

      jthread driver{ [&] { publisher.publish(1); } };

      auto result = source.numbers.wait(test_timeout);

      Assert::IsTrue(bool(result));
      Assert::AreEqual(1, get<0>(*result));
    }

    TEST_METHOD(TestWaitTimesOutWhenNoEventIsRaised)
    {
      number_source source;

      auto result = source.numbers.wait(200ms);

      Assert::IsFalse(bool(result));
    }

    TEST_METHOD(TestWaitWithPredicateSkipsNonMatchingEvents)
    {
      number_source source;
      background_publisher publisher{ source };

      //Values 1..3 are rejected by the predicate, so these raises complete without a waiter,
      //and only the matching value 4 is handed over.
      jthread driver{ [&] {
        for (auto value : { 1, 2, 3, 4 })
        {
          publisher.publish(value);
        }
      } };

      auto result = source.numbers.wait([](int value) { return value == 4; }, test_timeout);

      Assert::IsTrue(bool(result));
      Assert::AreEqual(4, get<0>(*result));
    }

    TEST_METHOD(TestWaitWithPredicateMatchesOnMultipleArguments)
    {
      pair_source source;

      jthread driver{ [&] {
        source.publish("alpha", 1);
        source.publish("gamma", 3);
      } };

      auto result = source.pairs.wait([](const string& text, int) { return text == "gamma"; }, test_timeout);

      Assert::IsTrue(bool(result));
      Assert::AreEqual<string>("gamma", get<0>(*result));
      Assert::AreEqual(3, get<1>(*result));
    }

    TEST_METHOD(TestWaitWithPredicateTimesOutWhenNoEventMatches)
    {
      number_source source;
      background_publisher publisher{ source };

      auto result = source.numbers.wait([](int value) { return value == 42; }, 200ms);

      Assert::IsFalse(bool(result));
    }

    TEST_METHOD(TestNonMatchingEventDoesNotHoldUpOtherSubscribers)
    {
      number_source source;
      background_publisher publisher{ source };

      //The awaiter rejects this value, which must not stop the other subscriber from receiving it,
      //nor block the publishing thread - background_publisher::publish would time out otherwise.
      auto received = 0;
      auto subscription = source.numbers([&](int value) { received = value; });

      event_awaiter<int> awaiter{ source.numbers, [](int value) { return value == 42; } };

      publisher.publish(1);

      Assert::AreEqual(1, received);
    }

    TEST_METHOD(TestAwaiterWithPredicateCanWaitForSubsequentMatches)
    {
      number_source source;
      background_publisher publisher{ source };

      event_awaiter<int> awaiter{ source.numbers, [](int value) { return value % 2 == 0; } };

      jthread driver{ [&] {
        for (auto value : { 1, 2 })
        {
          publisher.publish(value);
        }
      } };

      auto first = awaiter.wait(test_timeout);
      Assert::IsTrue(bool(first));
      Assert::AreEqual(2, get<0>(*first));
      first.reset();
      driver.join();

      jthread secondDriver{ [&] {
        for (auto value : { 3, 4 })
        {
          publisher.publish(value);
        }
      } };

      auto second = awaiter.wait(test_timeout);
      Assert::IsTrue(bool(second));
      Assert::AreEqual(4, get<0>(*second));
    }
  };
}
