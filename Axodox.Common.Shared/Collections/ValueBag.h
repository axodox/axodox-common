#pragma once
#include "common_includes.h"

namespace Axodox::Collections
{
  class value_bag
  {
    using deleter_t = void(*)(void*);

    struct item_t {
      void* value;
      deleter_t deleter;
    };

    template<typename T>
    static void delete_value(T* value)
    {
      delete static_cast<T*>(value);
    }

  public:
    template<typename T, typename... TArgs>
    T& add(TArgs&&... args)
    {
      item_t item{
        .value = new T(std::forward<TArgs>(args)...),
        .deleter = &delete_value<T>
      };

      _values.push_back(item);
      return *static_cast<T*>(item.value);
    }

    template<typename T>
    T& add(T&& value)
    {
      item_t item{
        .value = new T(std::move(value)),
        .deleter = &delete_value<T>
      };

      _values.push_back(item);
      return *static_cast<T*>(item.value);
    }

    ~value_bag()
    {
      for (auto item : _values)
      {
        item.deleter(item.value);
      }
    }

  private:
    std::vector<item_t> _values;
  };
}