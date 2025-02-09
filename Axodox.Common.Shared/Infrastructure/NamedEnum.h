#pragma once
#include "common_includes.h"

#define named_enumeration(flags, type, ...)                                                              \
  enum class type { __VA_ARGS__ };                                                                       \
  inline const Axodox::Infrastructure::named_enum_serializer __NAMED_ENUM_##type{ #__VA_ARGS__, flags }; \
                                                                                                         \
  template<> std::string_view to_string(type value) {                                                    \
    return __NAMED_ENUM_##type.to_string(uint64_t(value));                                               \
  }                                                                                                      \
                                                                                                         \
  template<> type to_value(std::string_view name) {                                                      \
    return type(__NAMED_ENUM_##type.to_value(name));                                                     \
  }

#define named_enum(type, ...) named_enumeration(false, type, __VA_ARGS__)
#define named_flags(type, ...) named_enumeration(true, type, __VA_ARGS__)

namespace Axodox::Infrastructure
{
  class named_enum_serializer
  {
    struct enum_value
    {
      std::string name;
      uint64_t value;
    };

  public:
    named_enum_serializer(const std::string_view items, bool flags = false);

    std::string_view to_string(uint64_t value) const;
    uint64_t to_value(std::string_view name) const;

  private:
    std::vector<enum_value> _items;
  };

  template<typename T>
    requires std::is_enum_v<T>
  std::string_view to_string(T value)
  {
    static_assert(false, "The specified type is not declared as a named enumeration.");
  }

  template<typename T>
    requires std::is_enum_v<T>
  T to_value(std::string_view name)
  {
    static_assert(false, "The specified type is not declared as a named enumeration.");
  }
}