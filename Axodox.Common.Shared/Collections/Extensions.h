#pragma once
#include "Infrastructure/Concepts.h"

using namespace std;

namespace Axodox::Collections
{
  template<typename T>
  std::unique_ptr<T> remove_unordered(std::vector<std::unique_ptr<T>>& collection, const T* item)
  {
    auto it = std::ranges::find_if(collection, [&collection, item](const std::unique_ptr<T>& p) { return p.get() == item; });
    if (it == collection.end()) return nullptr;

    if (it + 1 != collection.end())
    {
      std::swap(*it, *collection.rbegin());
    }

    auto result = move(collection.back());
    collection.pop_back();
    return result;
  }
}