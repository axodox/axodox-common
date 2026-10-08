#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  struct destruction_tracker
  {
    bool* is_destroyed;

    ~destruction_tracker()
    {
      *is_destroyed = true;
    }
  };
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(NotSharedTests)
  {
  public:
    TEST_METHOD(TestReleasingDoesNotDestroy)
    {
      auto isDestroyed = false;
      {
        destruction_tracker tracker{ &isDestroyed };

        auto pointer = not_shared(tracker);
        Assert::IsTrue(pointer.get() == &tracker, L"the pointer does not point to the object");

        auto copy = pointer;
        pointer.reset();
        copy.reset();
        Assert::IsFalse(isDestroyed, L"releasing the last copy destroyed the object");
      }

      Assert::IsTrue(isDestroyed, L"the object was not destroyed by its owner");
    }

    TEST_METHOD(TestContainerResolvesTheWrappedObject)
    {
      auto isDestroyed = false;
      destruction_tracker tracker{ &isDestroyed };

      {
        dependency_container container;
        container.add<destruction_tracker>(not_shared(tracker));

        Assert::IsTrue(container.resolve<destruction_tracker>().get() == &tracker, L"the container resolved a different object");
      }

      Assert::IsFalse(isDestroyed, L"the container destroyed an object it does not own");
    }
  };
}
