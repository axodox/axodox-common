#pragma once
#include <bit>
#include "common_includes.h"
#include "Text.h"

#define named_enumeration(flags, type, ...)                                                              \
  enum class type { __VA_ARGS__ };                                                                       \
  inline const Axodox::Infrastructure::named_enum_serializer<type> __named_enum_##type{ #__VA_ARGS__, flags }; \
  \
  constexpr bool __is_named_enum_helper(type*) \
  { \
    return true; \
  };

#define named_enum(type, ...) named_enumeration(false, type, __VA_ARGS__)
#define named_flags(type, ...) named_enumeration(true, type, __VA_ARGS__)

namespace Axodox::Infrastructure
{
  template<typename T>
  constexpr bool __is_named_enum_helper(T*)
  {
    return false;
  }

  template<typename T>
  constexpr bool __is_named_enum()
  {
    return __is_named_enum_helper(static_cast<T*>(nullptr));
  }

  template<typename T>
  constexpr bool is_named_enum = __is_named_enum<T>();

  template<typename T>
  struct enum_value
  {
    std::string_view name;
    std::string key;
    T value;
  };

  template<typename T>
    requires std::is_enum_v<T>
  class named_enum_serializer
  {
  public:
    [[deprecated("Use try_parse, which returns an empty optional for invalid text instead of this value.")]]
    inline static const T invalid_value = T(~0ull);

    static std::span<const enum_value<T>> items()
    {
      return _items;
    }

    //The items are the enum declaration as written: "Name" or "Name = value" entries, where a value is
    //a number, a ~ complement or a | combination of numbers and earlier names. An entry without a
    //value follows the previous one, as in C++.
    named_enum_serializer(const std::string_view items, bool flags = false)
    {
      _isFlags = flags;

      uint64_t next = 0;
      for (auto entry : split(items, ','))
      {
        auto separator = entry.find('=');
        auto name = trim(entry.substr(0, separator));
        if (name.empty()) continue;

        auto value = separator == std::string_view::npos ? next : evaluate_value_definition(entry.substr(separator + 1));
        _items.push_back({
          .name = name,
          .key = to_lower(name),
          .value = T(std::underlying_type_t<T>(value))
        });

        next = value + 1;
      }
    }

    //A flags value without a name of its own is written as its single bit names joined with " | ".
    static std::string to_string(T value)
    {
      for (auto& item : _items)
      {
        if (item.value == value) return std::string(item.name);
      }

      if (_isFlags)
      {
        auto remaining = to_underlying_type(value);

        std::string result;
        for (auto& item : _items)
        {
          auto bits = to_underlying_type(item.value);
          if (!std::has_single_bit(bits) || (remaining & bits) == 0) continue;

          if (!result.empty()) result += " | ";
          result += item.name;
          remaining &= ~bits;
        }

        if (remaining == 0 && !result.empty()) return result;
      }

      return std::format("{}", std::underlying_type_t<T>(value));
    }

    //A name (case-insensitive), a number, or for flags a | combination of these; empty when the text is none of them.
    static std::optional<T> try_parse(std::string_view name)
    {
      name = trim(name);
      if (name.empty()) return std::nullopt;

      if (_isFlags && name.find('|') != std::string_view::npos)
      {
        underlying_t result = 0;
        for (auto part : split(name, '|'))
        {
          auto value = try_parse(part);
          if (!value) return std::nullopt;

          result |= to_underlying_type(*value);
        }

        return T(result);
      }

      if (std::isdigit(name[0]))
      {
        std::underlying_type_t<T> value;
        auto [end, error] = std::from_chars(name.data(), name.data() + name.size(), value);
        if (error != std::errc{} || end != name.data() + name.size()) return std::nullopt;

        return T(value);
      }

      auto canonicalName = to_lower(name);
      for (auto& item : _items)
      {
        if (item.key == canonicalName) return item.value;
      }

      return std::nullopt;
    }

    [[deprecated("Use try_parse, T(~0ull) cannot be told apart from a value with all bits set.")]]
    static T to_value(std::string_view name)
    {
      return try_parse(name).value_or(T(~0ull));
    }

    static bool exists()
    {
      return !_items.empty();
    }

  private:
    using underlying_t = std::make_unsigned_t<std::underlying_type_t<T>>;

    inline static std::vector<enum_value<T>> _items;
    inline static bool _isFlags = false;

    static underlying_t to_underlying_type(T value)
    {
      return underlying_t(std::underlying_type_t<T>(value));
    }

    static uint64_t evaluate_value_definition(std::string_view expression)
    {
      uint64_t result = 0;
      for (auto operand : split(expression, '|'))
      {
        result |= evaluate_value_definition_operand(trim(operand));
      }
      return result;
    }

    static uint64_t evaluate_value_definition_operand(std::string_view operand)
    {
      if (operand.starts_with('~')) return ~evaluate_value_definition_operand(trim(operand.substr(1)));
      if (operand.starts_with('-')) return uint64_t(-int64_t(evaluate_value_definition_operand(trim(operand.substr(1)))));

      if (!operand.empty() && std::isdigit(operand[0]))
      {
        auto base = 10;
        if (operand.size() > 2 && operand[0] == '0' && (operand[1] == 'x' || operand[1] == 'X'))
        {
          operand.remove_prefix(2);
          base = 16;
        }

        uint64_t value;
        auto [end, error] = std::from_chars(operand.data(), operand.data() + operand.size(), value, base);
        if (error == std::errc{} && std::string_view(end, operand.data() + operand.size()).find_first_not_of("uUlL") == std::string_view::npos) return value;
      }
      else
      {
        for (auto& item : _items)
        {
          if (item.name == operand) return uint64_t(std::underlying_type_t<T>(item.value));
        }
      }

      throw std::logic_error(std::format("Unsupported value '{}' in the declaration of a named enum.", operand));
    }
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

  //Empty when the text is not a value of T; for an integral type the whole text must be a number.
  template<typename T>
    requires std::is_enum_v<T> || std::is_integral_v<T>
  std::optional<T> try_parse(std::string_view text)
  {
    if constexpr (std::is_enum_v<T>)
    {
      return named_enum_serializer<T>::try_parse(text);
    }
    else
    {
      T value;
      auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
      if (error != std::errc{} || end != text.data() + text.size()) return std::nullopt;

      return value;
    }
  }

  template<typename T>
    requires std::is_enum_v<T> || std::is_integral_v<T>
  [[deprecated("Use try_parse, T(~0ull) cannot be told apart from a value with all bits set.")]]
  T parse(std::string_view text)
  {
    if constexpr (std::is_enum_v<T>)
    {
      return named_enum_serializer<T>::try_parse(text).value_or(T(~0ull));
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

  template<typename T>
    requires is_named_enum<T>
  std::span<const enum_value<T>> enum_values()
  {
    return named_enum_serializer<T>::items();
  }
}

template<typename T>
  requires Axodox::Infrastructure::is_named_enum<T>
struct std::formatter<T, char> : std::formatter<std::string_view, char>
{
  template<typename context_t>
  auto format(T value, context_t& context) const
  {
    return std::formatter<std::string_view, char>::format(Axodox::Infrastructure::to_string(value), context);
  }
};

template<typename T>
  requires Axodox::Infrastructure::is_named_enum<T>
struct std::formatter<T, wchar_t> : std::formatter<std::wstring_view, wchar_t>
{
  template<typename context_t>
  auto format(T value, context_t& context) const
  {
    //Enum names are identifiers and the fallback is a number, so the text is always ASCII.
    auto text = Axodox::Infrastructure::to_string(value);
    return std::formatter<std::wstring_view, wchar_t>::format(std::wstring(text.begin(), text.end()), context);
  }
};