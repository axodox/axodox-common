#pragma once
#include "JsonValue.h"

namespace Axodox::Json
{
  struct AXODOX_COMMON_API json_array : public json_value_container<std::vector<Infrastructure::value_ptr<json_value>>, json_type::array>
  {
    using json_array_iterator = std::vector<Infrastructure::value_ptr<json_value>>::const_iterator;

    using json_value_container::json_value_container;
    using json_value::to_string;

    Infrastructure::value_ptr<json_value>& operator[](size_t index);
    const Infrastructure::value_ptr<json_value>& operator[](size_t index) const;

    virtual void to_string(std::stringstream& stream) const override;
    static Infrastructure::value_ptr<json_array> from_string(std::string_view& text);

    json_array_iterator begin() const;
    json_array_iterator end() const;
  };

  template <typename value_t>
  requires Infrastructure::is_instantiation_of<std::vector, value_t>::value
  struct json_serializer<value_t>
  {
    static Infrastructure::value_ptr<json_value> to_json(value_t value)
    {
      auto json = Infrastructure::make_value<json_array>();
      json->value.reserve(value.size());

      for (auto& item : value)
      {
        json->value.push_back(json_serializer<value_t::value_type>::to_json(item));
      }

      return json;
    }

    static bool from_json(const json_value* json, value_t& value)
    {
      if (json && json->type() == json_type::array)
      {
        auto jsonArray = static_cast<const json_array*>(json);

        value.reserve(jsonArray->value.size());
        for (auto& item : *jsonArray)
        {
          typename value_t::value_type element;
          if (json_serializer<value_t::value_type>::from_json(item.get(), element))
          {
            value.push_back(std::move(element));
          }
        }

        return true;
      }
      else
      {
        return false;
      }
    }
  };
}