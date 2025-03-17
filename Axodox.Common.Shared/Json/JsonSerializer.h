#pragma once
#include "JsonObject.h"
#include "JsonString.h"
#include "JsonBoolean.h"
#include "JsonNumber.h"
#include "JsonArray.h"
#include "JsonNull.h"
#include "Infrastructure/NamedEnum.h"
#include "Infrastructure/Traits.h"
#include "Infrastructure/TypeRegistry.h"

namespace Axodox::Json
{
  class AXODOX_COMMON_API json_property_base
  {
  public:
    json_property_base(const char* name);

    const char* name() const;

    virtual Infrastructure::value_ptr<json_value> to_json() const = 0;
    virtual bool from_json(const json_value* json) = 0;

    virtual ~json_property_base() = default;

  private:
    const char* _name;
  };

  class AXODOX_COMMON_API json_object_base
  {
    template<typename value_t> friend class json_property;
    template<typename value_t> friend struct json_serializer;

  public:
    virtual ~json_object_base() = default;

  private:
    std::vector<ptrdiff_t> _propertyOffsets;
  };

  template<typename value_t>
  class json_property : json_property_base
  {
  public:
    json_property(json_object_base* owner, const char* name, value_t value = value_t{}) :
      json_property_base(name),
      _value(value)
    {
      owner->_propertyOffsets.push_back(intptr_t(this) - intptr_t(owner));
    }

    value_t& operator*()
    {
      return _value;
    }

    const value_t& operator*() const
    {
      return _value;
    }

    value_t* get()
    {
      return &_value;
    }

    const value_t* get() const
    {
      return &_value;
    }

    value_t* operator->()
    {
      return &_value;
    }

    const value_t* operator->() const
    {
      return &_value;
    }

    virtual Infrastructure::value_ptr<json_value> to_json() const override
    {
      return json_serializer<value_t>::to_json(_value);
    }

    virtual bool from_json(const json_value* json) override
    {
      return json_serializer<value_t>::from_json(json, _value);
    }

  private:
    value_t _value;
  };

  template <typename value_t>
    requires std::is_base_of_v<json_object_base, value_t>
  struct json_serializer<value_t>
  {
    static Infrastructure::value_ptr<json_value> to_json(const value_t& value, const std::function<void(json_object*)>& initializer = nullptr)
    {
      auto result = Infrastructure::make_value<json_object>();
      
      if (initializer)
      {
        initializer(result.get());
      }

      for (auto propertyOffset : value._propertyOffsets)
      {
        auto property = (const json_property_base*)(intptr_t(&value) + propertyOffset);
        result->set_value(property->name(), property->to_json());
      }

      return result;
    }

    static bool from_json(const json_value* json, value_t& value)
    {
      if (!json || json->type() != json_type::object) return false;

      auto jsonObject = static_cast<const json_object*>(json);

      for (auto propertyOffset : value._propertyOffsets)
      {
        auto property = (json_property_base*)((uint8_t*)&value + propertyOffset);

        json_value* propertyValue;
        if (jsonObject->try_get_value(property->name(), propertyValue))
        {
          property->from_json(propertyValue);
        }
      }

      return true;
    }
  };

  template <typename value_t>
    requires Infrastructure::is_pointing<value_t> && std::derived_from<Infrastructure::pointed_t<value_t>, json_object_base> && Infrastructure::has_derived_types<Infrastructure::pointed_t<value_t>>
  struct json_serializer<value_t>
  {
    using type = Infrastructure::pointed_t<value_t>;

    static Infrastructure::value_ptr<json_value> to_json(const value_t& value)
    {
      auto typeKey = type::derived_types.get_key(&*value);
      auto typeName = Infrastructure::to_string(typeKey);

      auto result = json_serializer<type>::to_json(*value, [&](json_object* object) {
        object->set_value("$type", typeName);
        });

      return result;
    }

    static bool from_json(const json_value* json, value_t& value)
    {
      if constexpr (std::is_pointer_v<value_t>) return false;
      if (!json || json->type() != json_type::object) return false;

      auto jsonObject = static_cast<const json_object*>(json);
      
      json_value* typeValue;
      if (!jsonObject->try_get_value("$type", typeValue) || typeValue->type() != json_type::string)
      {
        return false;
      }

      auto typeName = static_cast<json_string*>(typeValue)->value;
      auto typeKey = Infrastructure::parse<decltype(type::derived_types)::key_type>(typeName);

      value = type::derived_types.create_unique(uint32_t(typeKey));
      if (!value) return false;

      return json_serializer<type>::from_json(json, *value);
    }
  };
}