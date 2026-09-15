#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  struct plain_item
  {
    int value = 0;
  };

  struct indexed_item
  {
    explicit indexed_item(int32_t index) :
      _index(index)
    { }

    int32_t index() const
    {
      return _index;
    }

  private:
    int32_t _index;
  };

  template <typename T>
  size_t count_items(registry<T>& registry)
  {
    registry_snapshot snapshot{ registry };
    return size_t(distance(snapshot.begin(), snapshot.end()));
  }
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(RegistryTests)
  {
  public:
    TEST_METHOD(TestAddRaisesItemAdded)
    {
      registry<plain_item> registry;

      auto item = make_shared<plain_item>();
      shared_ptr<plain_item> addedItem;
      auto subscription = registry.item_added([&](auto*, const auto& value) { addedItem = value; });

      auto token = registry.add(item);

      Assert::IsTrue(item == addedItem);
      Assert::AreEqual(size_t{ 1 }, count_items(registry));
    }

    TEST_METHOD(TestResettingTokenRemovesItem)
    {
      registry<plain_item> registry;

      auto item = make_shared<plain_item>();
      shared_ptr<plain_item> removedItem;
      auto subscription = registry.item_removed([&](auto*, const auto& value) { removedItem = value; });

      auto token = registry.add(item);
      Assert::AreEqual(size_t{ 1 }, count_items(registry));

      token.reset();

      Assert::IsTrue(item == removedItem);
      Assert::AreEqual(size_t{ 0 }, count_items(registry));
    }

    TEST_METHOD(TestTokenRemovesItemWhenLeavingScope)
    {
      registry<plain_item> registry;

      {
        auto token = registry.add(make_shared<plain_item>());
        Assert::AreEqual(size_t{ 1 }, count_items(registry));
      }

      Assert::AreEqual(size_t{ 0 }, count_items(registry));
    }

    TEST_METHOD(TestPermanentItemsAreNotRemoved)
    {
      registry<plain_item> registry;

      auto removedCount = 0;
      auto subscription = registry.item_removed([&](auto*, const auto&) { removedCount++; });

      registry.add_permanent(make_shared<plain_item>());

      Assert::AreEqual(0, removedCount);
      Assert::AreEqual(size_t{ 1 }, count_items(registry));
    }

    TEST_METHOD(TestItemsAreOrderedByIndex)
    {
      registry<indexed_item> registry;

      registry.add_permanent(make_shared<indexed_item>(3));
      registry.add_permanent(make_shared<indexed_item>(1));
      registry.add_permanent(make_shared<indexed_item>(2));

      vector<int32_t> indices;
      registry_snapshot snapshot{ registry };
      for (auto& item : snapshot) indices.push_back(item->index());

      Assert::IsTrue(vector<int32_t>({ 1, 2, 3 }) == indices);
    }

    TEST_METHOD(TestItemsWithEqualIndicesAreAllKept)
    {
      registry<indexed_item> registry;

      registry.add_permanent(make_shared<indexed_item>(1));
      registry.add_permanent(make_shared<indexed_item>(1));

      Assert::AreEqual(size_t{ 2 }, count_items(registry));
    }

    TEST_METHOD(TestHandlerCanReadRegistryWhileItemIsAdded)
    {
      registry<plain_item> registry;

      size_t observedCount = 0;
      auto subscription = registry.item_added([&](auto* sender, const auto&) { observedCount = count_items(*sender); });

      registry.add_permanent(make_shared<plain_item>());

      Assert::AreEqual(size_t{ 1 }, observedCount);
    }

    TEST_METHOD(TestHandlerCanReadRegistryWhileItemIsRemoved)
    {
      registry<plain_item> registry;

      size_t observedCount = SIZE_MAX;
      auto subscription = registry.item_removed([&](auto* sender, const auto&) { observedCount = count_items(*sender); });

      {
        auto token = registry.add(make_shared<plain_item>());
      }

      Assert::AreEqual(size_t{ 0 }, observedCount);
    }

    TEST_METHOD(TestConcurrentAddsAreSerialized)
    {
      const auto threadCount = 8;
      const auto itemsPerThread = 250;

      registry<plain_item> registry;
      atomic<int> addedCount = 0;
      auto subscription = registry.item_added([&](auto*, const auto&) { addedCount++; });

      vector<jthread> threads;
      for (auto i = 0; i < threadCount; i++)
      {
        threads.emplace_back([&] {
          for (auto j = 0; j < itemsPerThread; j++)
          {
            registry.add_permanent(make_shared<plain_item>());
          }
        });
      }
      threads.clear();

      Assert::AreEqual(threadCount * itemsPerThread, addedCount.load());
      Assert::AreEqual(size_t{ threadCount * itemsPerThread }, count_items(registry));
    }

    TEST_METHOD(TestConcurrentTokensRemoveAllItems)
    {
      const auto threadCount = 8;
      const auto itemsPerThread = 250;

      registry<plain_item> registry;

      {
        vector<jthread> threads;
        for (auto i = 0; i < threadCount; i++)
        {
          threads.emplace_back([&] {
            vector<lifetime_token> tokens;
            for (auto j = 0; j < itemsPerThread; j++)
            {
              tokens.push_back(registry.add(make_shared<plain_item>()));
            }
          });
        }
      }

      Assert::AreEqual(size_t{ 0 }, count_items(registry));
    }
  };
}
