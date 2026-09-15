#pragma once
#include "common_includes.h"
#include "Events.h"
#include "LifetimeToken.h"

namespace Axodox::Infrastructure
{
#ifdef PLATFORM_WINDOWS
  template <typename T>
  using registry_ptr = std::conditional_t<winrt::impl::has_category_v<T>, T, std::shared_ptr<T>>;
#else
  template <typename T>
  using registry_ptr = std::shared_ptr<T>;
#endif

  template <typename T>
  struct registry_comparer
  {
    bool operator()(const registry_ptr<T>& a, const registry_ptr<T>& b) const
    {
      if constexpr (requires(T value) { { value.index() } -> std::same_as<int32_t>; })
      {
        int32_t indexA;
        int32_t indexB;

        if constexpr (std::same_as<T, registry_ptr<T>>)
        {
          indexA = a.index();
          indexB = b.index();
        }
        else
        {
          indexA = a->index();
          indexB = b->index();
        }

        return (indexA != indexB) ? (indexA < indexB) : std::less<registry_ptr<T>>{}(a, b);
      }
      else
      {
        return std::less<registry_ptr<T>>{}(a, b);
      }
    }
  };

  template <typename T>
  class registry
  {
  public:
    using item_t = T;
    using items_t = std::set<registry_ptr<item_t>, registry_comparer<item_t>>;

    template <typename TItem>
    friend class registry_snapshot;

  private:
    std::shared_mutex _mutex;
    items_t _items;
    event_owner _events;

  public:
    event_publisher<registry*, const registry_ptr<item_t>&> item_added;
    event_publisher<registry*, const registry_ptr<item_t>&> item_removed;

    registry() :
      item_added(_events),
      item_removed(_events)
    { }

    explicit registry(items_t&& items) :
      registry()
    {
      _items = std::move(items);
    }

    [[nodiscard]] lifetime_token add(const registry_ptr<item_t>& item)
    {
      add_permanent(item);

      return lifetime_token([this, item] {
        {
          std::unique_lock lock(_mutex);
          _items.erase(item);
        }

        item_removed.raise(_events, this, item);
      });
    }

    void add_permanent(const registry_ptr<item_t>& item)
    {
      {
        std::unique_lock lock(_mutex);
        _items.emplace(item);
      }

      item_added.raise(_events, this, item);
    }
  };

  template <typename T>
  class registry_snapshot
  {
  public:
    using owner_t = registry<T>;

    explicit registry_snapshot(owner_t& owner) :
      _owner(&owner)
    {
      _owner->_mutex.lock_shared();
    }

    registry_snapshot(const registry_snapshot&) = delete;
    registry_snapshot& operator=(const registry_snapshot&) = delete;

    typename owner_t::items_t::const_iterator begin() const
    {
      return _owner->_items.begin();
    }

    typename owner_t::items_t::const_iterator end() const
    {
      return _owner->_items.end();
    }

    ~registry_snapshot()
    {
      _owner->_mutex.unlock_shared();
    }

  private:
    owner_t* const _owner;
  };
}
