#pragma once
#include "Infrastructure/Concepts.h"
#include "JsonValue.h"
#include "JsonString.h"
#include "JsonNumber.h"
#include "JsonBoolean.h"

namespace Axodox::Json
{
  struct AXODOX_COMMON_API json_object : public json_value_container<std::unordered_map<std::string, Infrastructure::value_ptr<json_value>>, json_type::object>
  {
    using json_value_container::json_value_container;
    using json_value::to_string;

    Infrastructure::value_ptr<json_value>& operator[](const char* property);
    const Infrastructure::value_ptr<json_value>& operator[](const char* property) const;

    Infrastructure::value_ptr<json_value>& at(const char* property);
    const Infrastructure::value_ptr<json_value>& at(const char* property) const;

    virtual bool is_default() const override;

    virtual void to_string(json_stream& stream) const override;
    static Infrastructure::value_ptr<json_object> from_string(std::string_view& text);

    void set_value(const char* key, Infrastructure::value_ptr<json_value>&& json)
    {
      this->value[key] = std::move(json);
    }

    json_value* get_value(const char* key) const
    {
      return value.at(key).get();
    }

    bool try_get_value(const char* key, json_value*& json) const
    {
      auto it = this->value.find(key);
      if (it == this->value.end()) return false;

      json = it->second.get();
      return true;
    }

    template<typename value_t>
    void set_value(const char* key, const value_t& value)
    {
      this->value[key] = json_serializer<value_t>::to_json(value);
    }

    template<typename value_t>
    void get_value(const char* key, value_t& value) const
    {
      json_serializer<value_t>::from_json(at(key).get(), value);
    }

    template<typename value_t>
    value_t get_value(const char* key) const
    {
      value_t value;
      get_value(key, value);
      return value;
    }

    template<typename value_t>
    bool try_get_value(const char* key, value_t& json) const
    {
      auto it = this->value.find(key);
      if (it == this->value.end()) return false;

      json_serializer<value_t>::from_json(it->second.get(), json);
      return true;
    }
  };

  template<>
  struct AXODOX_COMMON_API json_serializer<json_object>
  {
    static Infrastructure::value_ptr<json_value> to_json(const json_object& value);

    static bool from_json(const json_value* json, json_object& value);

    static bool is_default(const json_object& value);
  };

  //The keys a map may have in json: strings as they are, and named enums by their names
  template<typename key_t>
  concept json_map_key = std::same_as<key_t, std::string> || Infrastructure::is_named_enum<key_t>;

  //A map keyed by strings or named enums, which is written as a json object with its keys as the property names
  template<typename value_t>
  concept json_map = (Infrastructure::instantiation_of<value_t, std::unordered_map> || Infrastructure::instantiation_of<value_t, std::map>)
    && json_map_key<typename value_t::key_type>;

  template<json_map value_t>
  struct json_serializer<value_t>
  {
    using key_t = typename value_t::key_type;
    using mapped_t = typename value_t::mapped_type;

    static Infrastructure::value_ptr<json_value> to_json(const value_t& value)
    {
      auto json = Infrastructure::make_value<json_object>();
      json->value.reserve(value.size());

      for (auto& [key, item] : value)
      {
        if constexpr (std::same_as<key_t, std::string>)
        {
          json->set_value(key.c_str(), json_serializer<mapped_t>::to_json(item));
        }
        else
        {
          json->set_value(Infrastructure::to_string(key).c_str(), json_serializer<mapped_t>::to_json(item));
        }
      }

      return json;
    }

    //Like an array drops the items it cannot read, the entries with a key or a value which cannot be read are left out
    static bool from_json(const json_value* json, value_t& value)
    {
      if (!json || json->type() != json_type::object) return false;

      value.clear();
      for (auto& [name, item] : static_cast<const json_object*>(json)->value)
      {
        key_t key;
        if constexpr (std::same_as<key_t, std::string>)
        {
          key = name;
        }
        else
        {
          auto parsedKey = Infrastructure::try_parse<key_t>(name);
          if (!parsedKey) continue;

          key = *parsedKey;
        }

        mapped_t element;
        if (json_serializer<mapped_t>::from_json(item.get(), element))
        {
          value.emplace(std::move(key), std::move(element));
        }
      }

      return true;
    }

    static bool is_default(const value_t& value)
    {
      return value.empty();
    }
  };
}