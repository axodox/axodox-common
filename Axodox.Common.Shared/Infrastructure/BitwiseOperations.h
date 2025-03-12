#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  template<typename T>
  constexpr T bitwise_or(T a, T b)
  {
    return (T)(static_cast<std::underlying_type_t<T>>(a) | static_cast<std::underlying_type_t<T>>(b));
  }

  template<typename T>
  constexpr T bitwise_and(const T a, const T b)
  {
    return (T)(static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b));
  }

  template<typename T>
  constexpr T bitwise_negate(const T a)
  {
    return (T)(~static_cast<std::underlying_type_t<T>>(a));
  }

  template<typename T>
  constexpr bool has_flag(const T a, const T b)
  {
    return bitwise_and(a, b) == b;
  }

  template<typename T>
  constexpr bool has_any_flag(const T a, const T b)
  {
    return bitwise_and(a, b) != T(0);
  }

  template<typename T>
  void add_flag(T& a, const T b, const bool value = true)
  {
    if (value)
    {
      a = bitwise_or(a, b);
    }
  }

  template<typename T>
  void set_flag(T& a, const T b, const bool value)
  {
    if (value)
    {
      a = bitwise_or(a, b);
    }
    else
    {
      a = bitwise_and(a, bitwise_negate(b));
    }
  }

  template <typename T>
  void zero_memory(T& value)
  {
    static_assert(std::is_trivially_copyable_v<T>, "Cannot zero out non-trivially copiable types.");

    memset(&value, 0, sizeof(T));
  }

  template <typename T>
  bool are_equal(const T& a, const T& b)
  {
    if constexpr (std::is_trivially_copyable_v<T>)
    {
      return memcmp(&a, &b, sizeof(T)) == 0;
    }
    else
    {
      return a == b;
    }
  }

  template <typename T>
  bool are_equal(std::span<const T> a, std::span<const T> b)
  {
    static_assert(std::is_trivially_copyable<T>::value);

    if (a.size() != b.size()) return false;
    return memcmp(a.data(), b.data(), sizeof(T) * a.size()) == 0;
  }

  template<typename U, typename V, std::enable_if_t<std::conjunction_v<std::is_trivially_copyable<U>, std::is_trivially_copyable<V>>> = true>
  constexpr bool are_equal(const U& a, const V& b)
  {
    static_assert(sizeof(U) == sizeof(V));
    return memcmp(&a, &b, sizeof(U)) == 0;
  }

#ifdef WINRT_Windows_Foundation_H
  AXODOX_COMMON_API bool are_equal(const winrt::Windows::Foundation::IInspectable& a, const winrt::Windows::Foundation::IInspectable& b);
#endif

  template <typename T>
  bool is_default(const T& value)
  {
    return are_equal<T>(value, T{});
  }

  template<typename T>
  std::span<const uint8_t> to_span(const T& value)
  {
    return { reinterpret_cast<const uint8_t*>(&value), sizeof(T) };
  }

  template<typename T>
  std::span<const uint8_t> to_span(std::span<const T> span)
  {
    return { reinterpret_cast<const uint8_t*>(span.data()), sizeof(T) * span.size() };
  }

  template<typename T>
  std::span<const uint8_t> to_span(std::initializer_list<T> list)
  {
    return { reinterpret_cast<const uint8_t*>(list.begin()), reinterpret_cast<const uint8_t*>(list.end()) };
  }

  template<typename T>
  std::vector<uint8_t> to_vector(std::span<const T> span)
  {
    return { reinterpret_cast<const uint8_t*>(span.data()), sizeof(T) * span.size() };
  }

  template<typename T>
  std::vector<uint8_t> to_vector(std::initializer_list<T> list)
  {
    return { reinterpret_cast<const uint8_t*>(list.begin()), reinterpret_cast<const uint8_t*>(list.end()) };
  }
}