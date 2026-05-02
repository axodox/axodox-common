#pragma once
#include "Infrastructure/VoidPtr.h"
#include "Infrastructure/NamedEnum.h"
#include "JsonSerializer.h"

namespace Axodox::Json
{
#pragma region Type metadata
  template<typename value_t>
  struct json_type_metadata
  {
    using type = void;
  };

  template<typename value_t>
  using json_schema_type = json_type_metadata<value_t>::type;

  struct json_object_schema_base
  { };

  template<typename schema_t, json_type type_v>
  struct json_type_schema : public json_object_schema_base
  {
    json_type type = type_v;
    //Members here cannot be aggregate initialized directly

    void populate_schema(json_object& schema) const
    {
      //Do nothing, shadow in derived classes for static dispatch
    }

    Infrastructure::value_ptr<json_value> to_json() const
    {
      auto result = Infrastructure::make_value<json_object>();
      result->set_value("type", Infrastructure::to_string(type));
      static_cast<const schema_t*>(this)->populate_schema(*result.get());
      return result;
    }
  };
#pragma endregion

#pragma region Object description
  template<typename object_t>
  class json_object_descriptor;

  template<typename value_t>
  struct json_object_schema;

  class json_property_descriptor_base
  {
  public:
    json_type type() const { return _type; }
    const char* name() const { return _name; }

    Infrastructure::value_ptr<json_value> to_json(const void* object) const 
    { 
      return _serialize(object); 
    }

    bool from_json(void* object, const json_value* json) const 
    { 
      return _deserialize(object, json); 
    }
    
    const json_object_schema_base* schema() const 
    { 
      return static_cast<const json_object_schema_base*>(_schema.get()); 
    }

    Infrastructure::value_ptr<json_value> to_json_schema() const 
    {
      return _describe(_schema.get()); 
    }

  protected:
    //Storage-only constructor; users go through json_property_descriptor<object_t> which enforces
    //the field's owner type at compile time. The void*-erased lambdas assume the field's owner
    //sits at offset 0 of whichever object_t is later passed in (single-inheritance hierarchies).
    template<typename object_t, typename value_t, typename converter_t = json_serializer<value_t>>
    json_property_descriptor_base(value_t object_t::* field, const char* name, const json_schema_type<value_t>& schema = {}, converter_t converter = {}) :
      _type(schema.type),
      _name(name),
      _serialize([=](const void* object) { return converter_t::to_json(static_cast<const object_t*>(object)->*field); }),
      _deserialize([=](void* object, const json_value* json) { return converter_t::from_json(json, static_cast<object_t*>(object)->*field); }),
      _describe([](const void* schema) { return static_cast<const json_schema_type<value_t>*>(schema)->to_json(); }),
      _schema(schema)
    { }

  private:
    json_type _type;
    const char* _name;
    std::function<Infrastructure::value_ptr<json_value>(const void*)> _serialize;
    std::function<bool(void*, const json_value*)> _deserialize;
    std::function<Infrastructure::value_ptr<json_value>(const void*)> _describe;
    Infrastructure::void_ptr _schema;
  };

  template<typename object_t>
  class json_property_descriptor : public json_property_descriptor_base
  {
  public:
    template<typename value_t, typename converter_t = json_serializer<value_t>>
    json_property_descriptor(value_t object_t::* field, const char* name, const json_schema_type<value_t>& schema = {}, converter_t converter = {}) :
      json_property_descriptor_base(field, name, schema, converter)
    {
    }
  };

  template<typename object_t>
  concept described_json_object = requires { { object_t::json_description.properties() } -> std::convertible_to<const std::vector<json_property_descriptor_base>&>; };

  template<typename object_t>
  class json_object_descriptor : public json_object_schema<object_t>
  {
    template<typename other_t> friend class json_object_descriptor;

  public:
    using json_object_schema<object_t>::description;

    std::type_index index() const { return _index; }
    const char* name() const { return _name; }
    const std::vector<json_property_descriptor_base>& properties() const { return _properties; }
    const std::unordered_map<std::type_index, json_object_descriptor*>& base_descriptors() const { return _base_descriptors; }
    const std::unordered_map<std::type_index, json_object_descriptor*>& derived_descriptors() const { return _derived_descriptors; }
    std::unique_ptr<object_t> instantiate() const { return _instantiate ? _instantiate() : nullptr; }

    template<typename base_t>
    static json_object_descriptor create(const char* name, const char* description,
      std::initializer_list<json_property_descriptor<object_t>> properties)
    {
      json_object_descriptor result;
      result._name = name;
      result.description = description;
      result._instantiate = []() -> std::unique_ptr<object_t> { return std::make_unique<object_t>(); };

      if constexpr (described_json_object<base_t>)
      {
        //We rely on the base subobject sitting at offset 0 in object_t (single inheritance from base_t,
        //and base_t itself reachable through a chain of zero-offset bases). Multiple inheritance, or any
        //setup that places base_t at a non-zero offset, would break the void*-erased property lambdas.
        constexpr auto sample = std::uintptr_t{ alignof(object_t) };
        if (reinterpret_cast<std::uintptr_t>(static_cast<base_t*>(reinterpret_cast<object_t*>(sample))) != sample)
        {
          throw std::logic_error(std::format("JSON object descriptor for '{}' requires base '{}' at offset 0; multiple inheritance is not supported.", name, base_t::json_description._name));
        }

        add_derived(reinterpret_cast<json_object_descriptor*>(&base_t::json_description), &object_t::json_description);

        result._properties.reserve(properties.size() + base_t::json_description._properties.size());
        result._properties.insert(result._properties.end(), base_t::json_description._properties.begin(), base_t::json_description._properties.end());
      }

      result._properties.insert(result._properties.end(), properties.begin(), properties.end());
      return result;
    }

  private:
    std::type_index _index = std::type_index(typeid(object_t));
    const char* _name = nullptr;
    std::vector<json_property_descriptor_base> _properties;
    std::unordered_map<std::type_index, json_object_descriptor*> _base_descriptors;
    std::unordered_map<std::type_index, json_object_descriptor*> _derived_descriptors;
    std::unique_ptr<object_t>(*_instantiate)() = nullptr;

    static void add_derived(json_object_descriptor* base, json_object_descriptor* derived)
    {
      for (auto& [_, existing] : base->_derived_descriptors)
      {
        if (existing != derived && std::string_view(existing->_name) == derived->_name)
        {
          throw std::logic_error(std::format("Duplicate JSON object descriptor name '{}' under base '{}'.", derived->_name, base->_name));
        }
      }

      for (auto [index, indirectBase] : base->_base_descriptors)
      {
        add_derived(indirectBase, derived);
      }

      derived->_base_descriptors[base->_index] = base;
      base->_derived_descriptors[derived->_index] = derived;
    }

  public:
    using json_object_schema<object_t>::to_json;

    void to_json(const object_t& object, json_object* json) const
    {
      for (auto& property : _properties)
      {
        json->set_value(property.name(), property.to_json(&object));
      }
    }

    template<typename value_t>
      requires Infrastructure::is_pointing<value_t> && std::convertible_to<Infrastructure::pointed_t<value_t>*, object_t*>
    Infrastructure::value_ptr<json_value> to_json(const value_t& object) const
    {
      if (!object) return Infrastructure::make_value<json_null>();

      auto result = Infrastructure::make_value<json_object>();
      result->set_value("$type", _name);
      to_json(*object, result.get());
      return result;
    }

    //On a property failure, returns false and leaves the object partially mutated; caller should treat the value as invalid.
    bool from_json(object_t& object, const json_value* json) const
    {
      if (!json || json->type() != json_type::object) return false;

      auto jsonObject = static_cast<const json_object*>(json);
      for (auto& property : _properties)
      {
        json_value* jsonValue;
        if (jsonObject->try_get_value(property.name(), jsonValue))
        {
          if (!property.from_json(&object, jsonValue)) return false;
        }
      }

      return true;
    }

    template<typename result_t>
      requires requires(std::unique_ptr<object_t> o, result_t r) { r = std::move(o); }
    bool from_json(const json_value* json, result_t& result) const
    {
      if (!json) return false;
      if (json->type() == json_type::null)
      {
        result.reset();
        return true;
      }

      if (json->type() != json_type::object) return false;
      auto object = static_cast<const json_object*>(json);

      auto description = this;

      std::string type;
      if (object->try_get_value<std::string>("$type", type) && type != _name)
      {
        auto it = std::ranges::find_if(_derived_descriptors, [&type](const auto& value) { return value.second->_name == type; });
        if (it != _derived_descriptors.end())
        {
          description = it->second;
        }
      }

      result = description->_instantiate();
      return description->from_json(*result, json);
    }
  };

  template<typename object_t, typename base_t = void>
  json_object_descriptor<object_t> describe_json_object(
    const char* name,
    const char* description,
    std::initializer_list<json_property_descriptor<object_t>> properties)
  {
    return json_object_descriptor<object_t>::template create<base_t>(name, description, properties);
  }
#pragma endregion

#pragma region Type specific schemas
  struct json_number_schema : public json_type_schema<json_number_schema, json_type::number>
  {
    const char* description = nullptr;
    std::optional<double> minimum;
    std::optional<double> maximum;

    void populate_schema(json_object& schema) const
    {
      if (description) schema.set_value("description", description);
      if (minimum) schema.set_value("minimum", *minimum);
      if (maximum) schema.set_value("maximum", *maximum);
    }
  };

  struct json_boolean_schema : public json_type_schema<json_boolean_schema, json_type::boolean>
  {
    const char* description = nullptr;

    void populate_schema(json_object& schema) const
    {
      if (description) schema.set_value("description", description);
    }
  };

  template<typename enum_t>
    requires is_named_enum<enum_t>
  struct json_enum_schema : public json_type_schema<json_enum_schema<enum_t>, json_type::string>
  {
    const char* description = nullptr;

    void populate_schema(json_object& schema) const
    {
      if (description) schema.set_value("description", description);

      auto values = std::vector<std::string>();
      for (auto& value : Infrastructure::enum_values<enum_t>())
      {
        values.push_back(std::string(value.name));
      }
      schema.set_value("enum", values);
    }
  };

  struct json_string_schema : public json_type_schema<json_string_schema, json_type::string>
  {
    const char* description = nullptr;
    const char* pattern = nullptr;
    const char* format = nullptr;

    void populate_schema(json_object& schema) const
    {
      if (description) schema.set_value("description", description);
      if (pattern) schema.set_value("pattern", pattern);
      if (format) schema.set_value("format", format);
    }
  };

  template<typename item_t>
  struct json_array_schema : public json_type_schema<json_array_schema<item_t>, json_type::array>
  {
    const char* description = nullptr;
    std::optional<int32_t> min_items;
    std::optional<int32_t> max_items;
    json_schema_type<item_t> items;

    void populate_schema(json_object& schema) const
    {
      if (description) schema.set_value("description", description);
      if (min_items) schema.set_value("minItems", *min_items);
      if (max_items) schema.set_value("maxItems", *max_items);

      schema.set_value("items", items.to_json());
    }
  };

  template<typename object_t>
  struct json_object_schema : public json_type_schema<json_object_schema<object_t>, json_type::object>
  {
    const char* description = nullptr;
    const char* required = nullptr;

    void populate_schema(json_object& schema) const
    {
      if (description) schema.set_value("description", description);

      auto properties = Infrastructure::make_value<json_object>();
      if constexpr (described_json_object<object_t>)
      {
        const json_object_descriptor<object_t>& objectDescription = object_t::json_description;

        if (!description && objectDescription.description) schema.set_value("description", objectDescription.description);

        properties->value.reserve(objectDescription.properties().size());
        for (auto& property : objectDescription.properties())
        {
          properties->set_value(property.name(), property.to_json_schema());
        }
      }
      schema.set_value("properties", Infrastructure::value_ptr<json_value>(std::move(properties)));

      if (required) schema.set_value("required", Infrastructure::split(required, ','));
    }
  };
#pragma endregion

#pragma region Type metadata specializations
  template<>
  struct json_type_metadata<bool>
  {
    using type = json_boolean_schema;
  };

  template<typename value_t>
    requires std::is_arithmetic_v<value_t>
  struct json_type_metadata<value_t>
  {
    using type = json_number_schema;
  };

  template<>
  struct json_type_metadata<std::string>
  {
    using type = json_string_schema;
  };

  template<typename value_t>
  struct json_type_metadata<std::vector<value_t>>
  {
    using type = json_array_schema<value_t>;
  };

  template<std::derived_from<json_object_base> value_t>
  struct json_type_metadata<value_t>
  {
    using type = json_object_schema<value_t>;
  };

  template<typename value_t>
    requires is_named_enum<value_t>
  struct json_type_metadata<value_t>
  {
    using type = json_enum_schema<value_t>;
  };
#pragma endregion

#pragma region Serialization extensions
  template <typename value_t>
    requires described_json_object<value_t>
  struct json_serializer<value_t>
  {
    static json_object_descriptor<value_t>* get_type_description(const value_t& value)
    {
      auto id = std::type_index(typeid(value));
      auto result = &value_t::json_description;
      if (result->index() != id)
      {
        result = result->derived_descriptors().at(id);
      }
      return result;
    }

    static Infrastructure::value_ptr<json_value> to_json(const value_t& value, const std::function<void(json_object*)>& initializer = nullptr)
    {
      auto result = Infrastructure::make_value<json_object>();

      if (initializer)
      {
        initializer(result.get());
      }

      auto description = get_type_description(value);
      description->to_json(value, result.get());

      return result;
    }

    static bool from_json(const json_value* json, value_t& value)
    {
      if (!json || json->type() != json_type::object) return false;

      auto description = get_type_description(value);
      auto jsonObject = static_cast<const json_object*>(json);
      return description->from_json(value, jsonObject);
    }
  };

  template <typename value_t>
    requires Infrastructure::is_pointing<value_t>&& described_json_object<Infrastructure::pointed_t<value_t>>
  struct json_serializer<value_t>
  {
    using object_t = Infrastructure::pointed_t<value_t>;

    static Infrastructure::value_ptr<json_value> to_json(const value_t& value)
    {
      return json_serializer<object_t>::get_type_description(*value)->to_json(value);
    }

    static bool from_json(const json_value* json, value_t& value)
    {
      return object_t::json_description.from_json(json, value);
    }
  };
#pragma endregion
}