#pragma once
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

    virtual void to_string(std::stringstream& stream) const override;
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
      json_serializer<value_t>::from_json(at(key), value);
    }

    template<typename value_t>
    value_t get_value(const char* key) const
    {
      value_t value;
      get_value(key, value);
      return value;
    }
  };
}