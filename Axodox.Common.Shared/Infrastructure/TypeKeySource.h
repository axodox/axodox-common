#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  template<typename T>
  struct type_key_source
  {
    uint32_t operator()(const T* value)
    {
      static_assert(false, "This type is not supported by type_key_source");
    }
  };

  template<typename T>
  concept has_lowercase_type = requires(T value)
  {
    uint32_t(value.type());
  };

  template<typename T>
  concept has_uppercase_type = requires(T value)
  {
    uint32_t(value.Type());
  };

  template<typename T>
    requires has_lowercase_type<T> || has_uppercase_type<T>
  struct type_key_source<T>
  {
    auto operator()(const T* value) const
    {
      if constexpr (has_uppercase_type<T>)
      {
        return value->Type();
      }
      else
      {
        return value->type();
      }
    }
  };
}