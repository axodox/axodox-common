#include "common_includes.h"
#include "Include/Axodox.Threading.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Threading;
using namespace std;
using namespace std::chrono_literals;

namespace Axodox::Common::Tests
{
  TEST_CLASS(Win32EventsTests)
  {
  public:
    TEST_METHOD(TestManualResetEventStaysSetUntilReset)
    {
      win32_manual_reset_event event;
      Assert::IsFalse(bool(event));
      Assert::IsFalse(event.wait(10ms));

      event.set();
      Assert::IsTrue(event.wait(10ms));
      Assert::IsTrue(event.wait(10ms), L"waiting should not reset a manual reset event");
      Assert::IsTrue(bool(event));

      event.reset();
      Assert::IsFalse(bool(event));
    }

    TEST_METHOD(TestAutoResetEventResetsOnWait)
    {
      win32_auto_reset_event event{ true };
      Assert::IsTrue(event.wait(10ms));
      Assert::IsFalse(event.wait(10ms), L"waiting should reset an auto reset event");
    }

    TEST_METHOD(TestEventHandleCanBeWaitedOn)
    {
      win32_manual_reset_event first, second;
      second.set();

      HANDLE events[] = { first.get(), second.get() };
      Assert::AreEqual(DWORD(WAIT_OBJECT_0 + 1), WaitForMultipleObjects(2, events, FALSE, 0));
    }
  };
}
