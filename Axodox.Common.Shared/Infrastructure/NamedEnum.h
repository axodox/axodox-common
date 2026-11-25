#pragma once
#include "common_includes.h"
#include "Text.h"

//Pass of type* is for ADL (Argument-Dependent Lookup)

//Makes an enum declared elsewhere a named enum, place it in the namespace of the enum
//The items must repeat the enum declaration, such as for an enum with an underlying type or one declared by another macro
//The declaration is parsed on first use, so the names are available to any code, also during static initialization.
#define named_enum_declarations(flags, type, ...)                                                        \
  inline const Axodox::Infrastructure::typeless_enum_definition& __named_enum_definition(type*)          \
  {                                                                                                      \
    static const auto result = Axodox::Infrastructure::parse_named_enum_definition(#__VA_ARGS__, flags); \
    return result;                                                                                       \
  }                                                                                                      \
                                                                                                         \
  constexpr bool __is_named_enum_helper(type*)                                                           \
  {                                                                                                      \
    return true;                                                                                         \
  };

#define named_enumeration(flags, type, ...)                                                       \
  enum class type { __VA_ARGS__ };                                                                \
  named_enum_declarations(flags, type, __VA_ARGS__)

#define named_enum(type, ...) named_enumeration(false, type, __VA_ARGS__)
#define named_flags(type, ...) named_enumeration(true, type, __VA_ARGS__)

namespace Axodox::Infrastructure
{
  //A member of a named enum, with the bits of its value whatever the underlying type of the enum is
  struct typeless_enum_value
  {
    std::string name;
    std::string key; //The name in lower case, as names are parsed case-insensitively
    uint64_t value = 0;
  };

  //The members of a named enum, the named_enum macros parse them from the enum declaration
  struct AXODOX_COMMON_API typeless_enum_definition
  {
    std::vector<typeless_enum_value> items;
    bool is_flags = false;

    void add(std::string_view name, uint64_t value);
  };

  //Parses the enum declaration as written: "Name" or "Name = value" entries, where a value is a number,
  //a ~ complement or a | combination of numbers and earlier names. An entry without a value follows the previous one, as in C++.
  AXODOX_COMMON_API typeless_enum_definition parse_named_enum_definition(std::string_view declaration, bool is_flags);

  template<typename T>
  const typeless_enum_definition& __named_enum_definition(T*)
  {
    static const typeless_enum_definition empty;
    return empty;
  }

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

    //Refers to the name of the source, which must outlive the result
    static enum_value from_typeless(const typeless_enum_value& source)
    {
      return { .name = source.name, .key = source.key, .value = T(std::underlying_type_t<T>(source.value)) };
    }
  };

  template<typename T>
    requires std::is_enum_v<T>
  class named_enum_serializer
  {
    struct enum_definition
    {
      std::vector<enum_value<T>> items;
      bool is_flags = false;

      //Refers to the names of the source, which must outlive the result
      static enum_definition from_typeless(const typeless_enum_definition& source)
      {
        enum_definition result{ .is_flags = source.is_flags };
        result.items.reserve(source.items.size());
        for (auto& item : source.items)
        {
          result.items.push_back(enum_value<T>::from_typeless(item));
        }

        return result;
      }
    };

  public:
    [[deprecated("Use try_parse, which returns an empty optional for invalid text instead of this value.")]]
    inline static const T invalid_value = T(-1);

    static std::span<const enum_value<T>> items()
    {
      return get_definition().items;
    }

    //A flags value without a name of its own is written as its single bit names joined with " | ".
    static std::string to_string(T value)
    {
      auto& definition = get_definition();
      for (auto& item : definition.items)
      {
        if (item.value == value) return std::string(item.name);
      }

      if (definition.is_flags)
      {
        auto remaining = to_underlying_type(value);

        std::string result;
        for (auto& item : definition.items)
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

      auto& definition = get_definition();
      if (definition.is_flags && name.find('|') != std::string_view::npos)
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
      for (auto& item : definition.items)
      {
        if (item.key == canonicalName) return item.value;
      }

      return std::nullopt;
    }

    [[deprecated("Use try_parse, T(-1) cannot be told apart from a value with all bits set.")]]
    static T to_value(std::string_view name)
    {
      return try_parse(name).value_or(T(-1));
    }

    static bool exists()
    {
      return !get_definition().items.empty();
    }

  private:
    using underlying_t = std::make_unsigned_t<std::underlying_type_t<T>>;

    //The members of the enum with their values in its type, taken from its definition on first use
    static const enum_definition& get_definition()
    {
      static const enum_definition result = enum_definition::from_typeless(__named_enum_definition(static_cast<T*>(nullptr)));
      return result;
    }

    static underlying_t to_underlying_type(T value)
    {
      return underlying_t(std::underlying_type_t<T>(value));
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
  [[deprecated("Use try_parse, T(-1) cannot be told apart from a value with all bits set.")]]
  T parse(std::string_view text)
  {
    if constexpr (std::is_enum_v<T>)
    {
      return named_enum_serializer<T>::try_parse(text).value_or(T(-1));
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
        return T(-1);
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