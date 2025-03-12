#pragma once
#include "common_includes.h"
#include "Text.h"

#define named_enumeration(flags, type, ...)                                                              \
  enum class type { __VA_ARGS__ };                                                                       \
  inline const Axodox::Infrastructure::named_enum_serializer<type> __named_enum_##type{ #__VA_ARGS__, flags }; \

#define named_enum(type, ...) named_enumeration(false, type, __VA_ARGS__)
#define named_flags(type, ...) named_enumeration(true, type, __VA_ARGS__)

namespace Axodox::Infrastructure
{
  template<typename T>
    requires std::is_enum_v<T>
  class named_enum_serializer
  {
    struct enum_value
    {
      std::string_view name;
      std::string key;
      T value;
    };

  public:
    inline static const T invalid_value = T(~0ull);

    named_enum_serializer(const std::string_view items, bool flags = false)
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
          if (start != ~0u && end == ~0u) end = index;
        }

        if (index + 1 == items.size())
        {
          end = index + 1;
        }

        if (start != ~0u && end != ~0u)
        {
          enum_value item{
            .name = items.substr(start, end - start),
            .key = to_lower(item.name),
            .value = T(flags ? 1ull << _items.size() : _items.size())
          };

          _items.push_back(std::move(item));

          start = ~0u;
          end = ~0u;
        }

        index++;
      }
    }

    static std::string to_string(T value)
    {
      for (auto& item : _items)
      {
        if (item.value == value) return std::string(item.name);
      }

      return std::format("{}", std::underlying_type_t<T>(value));
    }

    static T to_value(std::string_view name)
    {
      if (name.empty()) return invalid_value;

      if (std::isdigit(name[0]))
      {
        std::underlying_type_t<T> value;
        if (std::from_chars(name.data(), name.data() + name.size(), value).ec == std::errc{})
        {
          return T(value);
        }
        return invalid_value;
      }
      else
      {
        auto canonicalName = to_lower(name);

        for (auto& item : _items)
        {
          if (item.key == canonicalName) return item.value;
        }
      }

      return invalid_value;
    }

    static bool exists()
    {
      return !_items.empty();
    }

  private:
    inline static std::vector<enum_value> _items;
  };

  template<typename T>
    requires std::is_enum_v<T> || std::is_integral_v<T>
  std::string to_string(T value)
  {
    if constexpr (std::is_enum_v<T>)
    {
      return std::string(named_enum_serializer<T>::to_string(value));
    }
    else
    {
      return std::format("{}", value);
    }
  }

  template<typename T>
    requires std::is_enum_v<T> || std::is_integral_v<T>
  T parse(std::string_view text)
  {
    if constexpr (std::is_enum_v<T>)
    {
      return named_enum_serializer<T>::to_value(text);
    }
    else
    {
      T value;
      if (std::from_chars(text.data(), text.data() + text.size(), value).ec == std::errc{})
      {
        return value;
      }
      else
      {
        return T(~0ull);
      }
    }
  }
}