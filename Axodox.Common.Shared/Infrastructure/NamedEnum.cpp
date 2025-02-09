#include "common_includes.h"
#include "NamedEnum.h"
#include "Text.h"

using namespace std;

namespace Axodox::Infrastructure
{
  named_enum_serializer::named_enum_serializer(std::string_view items, bool flags)
  {
    uint32_t start = ~0u;
    uint32_t end = ~0u;

    for (uint32_t index = 0u; auto item : items)
    {
      if (isalnum(item) || item == '_')
      {
        if (start == ~0u) start = index;
      }
      else
      {
        if (end == ~0u) end = index;
      }

      if (start != ~0u || end != ~0u)
      {
        _items.push_back(enum_value{ to_lower(items.substr(start, end - start)), flags ? 1ull << _items.size() : items.size() });

        start = ~0u;
        end = ~0u;
      }

      index++;
    }
  }

  std::string_view named_enum_serializer::to_string(uint64_t value) const
  {
    for (auto& item : _items)
    {
      if (item.value == value) return item.name;
    }

    return "";
  }

  uint64_t named_enum_serializer::to_value(std::string_view name) const
  {
    auto canonicalName = to_lower(name);

    for (auto& item : _items)
    {
      if (item.name == canonicalName) return item.value;
    }

    return ~0ull;
  }
}