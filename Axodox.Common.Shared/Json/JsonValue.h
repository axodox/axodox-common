#pragma once
#include "common_includes.h"
#include "Infrastructure/ValuePtr.h"
#include "Infrastructure/NamedEnum.h"

namespace Axodox::Json
{
  named_enum(json_type,
    null,
    boolean,
    number,
    string,
    array,
    object
  );

  template <typename value_t>
  struct json_serializer
  { };

  struct AXODOX_COMMON_API json_value
  {
    inline static const Infrastructure::value_ptr<json_value> empty = {};

    virtual ~json_value() = default;

    virtual json_type type() const = 0;
    virtual void to_string(std::stringstream& stream) const = 0;

    virtual bool is_default() const = 0;

    std::string to_string() const;

    static Infrastructure::value_ptr<json_value> from_string(std::string_view& text);
  };

  //A converter translates a C++ value to and from json, and decides whether a value counts as
  //default. Properties which are not required and hold a default value are omitted from the
  //output. json_serializer is the built in converter, custom ones are passed per property.
  template <typename converter_t, typename value_t>
  concept json_converter = requires (const value_t & value, const json_value * json, value_t & target)
  {
    { converter_t::to_json(value) } -> std::convertible_to<Infrastructure::value_ptr<json_value>>;
    { converter_t::from_json(json, target) } -> std::same_as<bool>;
    { converter_t::is_default(value) } -> std::same_as<bool>;
  };

  template<typename value_t, json_type json_type_c>
  struct json_value_container : public json_value
  {
    typedef value_t value_type;

    value_t value;

    json_value_container() :
      value(value_t{})
    { }

    json_value_container(const value_t& value) :
      value(std::move(value))
    { }

    json_value_container(value_t&& value) :
      value(std::move(value))
    { }

    operator const value_t& () const
    {
      return value;
    }

    virtual json_type type() const override
    {
      return json_type_c;
    }
  };

  AXODOX_COMMON_API void json_skip_whitespace(std::string_view& text);

  template<typename value_t>
  std::optional<value_t> try_parse_json(std::string_view text)
  {
    value_t result;

    auto json = json_value::from_string(text);
    if (json_serializer<value_t>::from_json(json.get(), result))
    {
      return result;
    }
    else
    {
      return std::nullopt;
    }
  }

  template<typename value_t>
  std::string stringify_json(const value_t& value)
  {
    return json_serializer<value_t>::to_json(value)->to_string();
  }

  template<>
  struct AXODOX_COMMON_API json_serializer<Infrastructure::value_ptr<json_value>>
  {
    static Infrastructure::value_ptr<json_value> to_json(const Infrastructure::value_ptr<json_value>& value);

    static bool from_json(const json_value* json, Infrastructure::value_ptr<json_value>& value);

    static bool is_default(const Infrastructure::value_ptr<json_value>& value);
  };

  //Null-safe wrapper over json_value::is_default(), for the common case of testing a
  //possibly empty value_ptr. A missing value counts as default.
  inline bool json_value_is_default(const json_value* value)
  {
    return !value || value->is_default();
  }
}