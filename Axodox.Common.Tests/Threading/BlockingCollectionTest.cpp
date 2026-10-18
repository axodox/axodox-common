#include "common_includes.h"
#include "Include/Axodox.Threading.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Threading;
using namespace std;

namespace Axodox::Common::Tests
{
  TEST_CLASS(BlockingCollectionTests)
  {
  public:
    TEST_METHOD(TestItemsAddedBeforeCompletionCanStillBeTaken)
    {
      blocking_collection<int> collection;
      collection.add(1);
      collection.add(2);
      collection.complete();

      int item = 0;
      Assert::IsTrue(collection.try_get(item));
      Assert::AreEqual(1, item);
      Assert::IsTrue(collection.try_get(item));
      Assert::AreEqual(2, item);
      Assert::IsFalse(collection.try_get(item), L"a complete, empty collection should not wait");
    }

    TEST_METHOD(TestAddingToACompleteCollection)
    {
      blocking_collection<int> collection;
      collection.complete();

      Assert::IsFalse(collection.try_add(1));
      Assert::ExpectException<logic_error>([&] { collection.add(1); });

      int item = 0;
      Assert::IsFalse(collection.try_get(item), L"the rejected items should not be queued");
    }
  };
}
