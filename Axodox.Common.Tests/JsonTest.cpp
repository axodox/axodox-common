#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.Json.h"

using namespace Axodox::Json;
using namespace Axodox::Infrastructure;
using namespace std;

template<typename value_t>
struct json_type_metadata
{
  using type = void;
};

template<typename value_t>
using json_schema_type = json_type_metadata<value_t>::type;

struct json_schema_variant;

using schema_generator = void (* const)(const json_schema_variant* that, json_object& schema);

struct json_schema_variant
{
  json_type type;
  schema_generator populate_schema;

  value_ptr<json_value> to_json() const
  {
    auto result = make_value<json_object>();
    result->set_value("type", to_string(type));
    populate_schema(this, *result.get());
    return result;
  }
};

template<typename schema_t, json_type type_v>
struct json_type_schema
{
  json_type type = type_v;
  schema_generator schema_source = [](const json_schema_variant* that, json_object& schema) {
    //Aggregate initialization does not support vtables, so use static dispatch
    reinterpret_cast<const schema_t*>(that)->populate_schema(schema);
    };

  //Members here cannot be aggregate initialized directly

  void populate_schema(json_object& schema) const
  {
    //Do nothing, shadow in derived classes for static dispactch
  }

  value_ptr<json_value> to_json() const
  {
    return reinterpret_cast<const json_schema_variant*>(this)->to_json();
  }
};

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

    auto values = vector<string>();
    for (auto& value : enum_values<enum_t>())
    {
      values.push_back(string(value.name));
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

template<typename value_t>
struct json_object_schema;

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

class void_value_ptr
{
  using copier = void* (*)(const void*);

  template<typename value_t>
    requires std::is_trivially_copyable_v<value_t>
  static void* copy_value(const void* value)
  {
    auto size = sizeof(value_t);
    auto result = malloc(size);
    memcpy(result, value, size);
    return result;
  }

public:
  void_value_ptr() :
    _copy(nullptr),
    _value(nullptr)
  {
  }

  template<typename value_t>
  void_value_ptr(const value_t& value) :
    _copy(&copy_value<value_t>),
    _value(_copy(&value))
  {
  }

  void_value_ptr(const void_value_ptr& other) :
    _copy(other._copy),
    _value(_copy(other._value))
  {
  }

  void_value_ptr& operator=(const void_value_ptr& other)
  {
    reset();

    _copy = other._copy;
    _value = _copy(other._value);

    return *this;
  }

  void_value_ptr(void_value_ptr&& other)
  {
    _copy = other._copy;
    _value = other._value;

    other._copy = nullptr;
    other._value = nullptr;
  }

  void_value_ptr& operator=(void_value_ptr&& other)
  {
    reset();

    _copy = other._copy;
    _value = other._value;

    other._copy = nullptr;
    other._value = nullptr;

    return *this;
  }

  ~void_value_ptr()
  {
    reset();
  }

  void reset()
  {
    if (!_value) return;

    free(_value);
    _value = nullptr;

    _copy = nullptr;
  }

  void* get() const
  {
    return _value;
  }

private:
  copier _copy;
  void* _value;
};

template<typename object_t>
class json_property_descriptor
{
  template<typename value_t>
  using typed_field_ptr = value_t(object_t::*);

public:
  const char* name;
  std::function<value_ptr<json_value>(const object_t&)> to_json;
  std::function<bool(object_t&, const json_value*)> from_json;

  const json_schema_variant* schema() const
  {
    return reinterpret_cast<const json_schema_variant*>(_schema.get());
  }

  template<typename value_t, typename converter_t = json_serializer<value_t>>
  json_property_descriptor(typed_field_ptr<value_t> field, const char* name, const json_schema_type<value_t>& schema = {}, converter_t converter = {}) :
    name(name),
    to_json([=](const object_t& object) { return converter_t::to_json(object.*field); }),
    from_json([=](object_t& object, const json_value* json) { return converter_t::from_json(json, object.*field); }),
    _schema(schema)
  {
  }

private:
  void_value_ptr _schema;
};

template<typename object_t>
struct json_object_descriptor;

template<typename object_t>
concept described_json_object = requires { { object_t::json_description.properties } -> std::convertible_to<vector<json_property_descriptor<object_t>>>; };

template<typename object_t>
struct json_object_descriptor : public json_object_schema<object_t>
{
  std::type_index index = type_index(typeid(object_t));
  const char* name;
  
  std::vector<json_property_descriptor<object_t>> properties;
  std::unordered_map<std::type_index, json_object_descriptor*> base_descriptors;
  std::unordered_map<std::type_index, json_object_descriptor*> derived_descriptors;
  std::function<unique_ptr<object_t>()> instantiate;

  template<typename base_t>
  static json_object_descriptor create(const char* name, const char* description,
    std::initializer_list<json_property_descriptor<object_t>> properties)
  {
    json_object_descriptor result;
    result.name = name;
    result.description = description;
    result.instantiate = [] { return make_unique<object_t>(); };

    if constexpr (described_json_object<base_t>)
    {
      add_derived(reinterpret_cast<json_object_descriptor*>(&base_t::json_description), &object_t::json_description);

      result.properties.reserve(properties.size() + base_t::json_description.properties.size());
      for (auto& inheritedProperty : base_t::json_description.properties)
      {
        result.properties.push_back(reinterpret_cast<const json_property_descriptor<object_t>&>(inheritedProperty));
      }
    }

    result.properties.insert(result.properties.end(), properties);
    return result;
  }

  static void add_derived(json_object_descriptor* base, json_object_descriptor* derived)
  {
    for (auto [index, indirectBase] : base->base_descriptors)
    {
      add_derived(indirectBase, derived);
    }

    derived->base_descriptors[base->index] = base;
    base->derived_descriptors[derived->index] = derived;
  }

  using json_object_schema<object_t>::to_json;

  void to_json(const object_t& object, json_object* json) const
  {
    for (auto& property : properties)
    {
      json->set_value(property.name, property.to_json(object));
    }
  }

  template<typename value_t>
  //  requires assignable_from<object_t, pointed_t<value_t>*>
  value_ptr<json_value> to_json(const value_t& object) const
  {
    if (!object) return make_value<json_null>();

    auto result = make_value<json_object>();
    result->set_value("$type", name);
    to_json(*object, result.get());
    return result;
  }

  bool from_json(object_t& object, const json_value* json) const
  {
    if (json->type() != json_type::object) return false;

    auto jsonObject = static_cast<const json_object*>(json);
    for (auto& property : properties)
    {
      json_value* jsonValue;
      if (jsonObject->try_get_value(property.name, jsonValue))
      {
        if (!property.from_json(object, jsonValue)) return false;
      }
    }

    return true;
  }

  template<typename result_t>
    requires requires(unique_ptr<object_t> o, result_t r) { r = move(o); }
  bool from_json(const json_value* json, result_t& result) const
  {
    if (json->type() == json_type::null)
    {
      result.reset();
      return true;
    }

    if (json->type() != json_type::object) return false;
    auto object = static_cast<const json_object*>(json);
    
    auto description = this;

    string type;
    if (object->try_get_value<string>("$type", type) && type != name)
    {
      auto it = ranges::find_if(derived_descriptors, [type](const auto& value) { return value.second->name == type; });
      if (it != derived_descriptors.end())
      {
        description = it->second;
      }
    }

    result = description->instantiate();
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

template<typename object_t>
struct json_object_schema : public json_type_schema<json_object_schema<object_t>, json_type::object>
{
  const char* description = nullptr;
  const char* required = nullptr;

  void populate_schema(json_object& schema) const
  {
    if (description) schema.set_value("description", description);

    auto properties = make_value<json_object>();
    if constexpr (described_json_object<object_t>)
    {
      const json_object_descriptor<object_t>& objectDescription = object_t::json_description;

      if (!description && objectDescription.description) schema.set_value("description", objectDescription.description);

      properties->value.reserve(objectDescription.properties.size());
      for (auto& property : objectDescription.properties)
      {
        properties->set_value(property.name, property.schema()->to_json());
      }
    }
    schema.set_value("properties", value_ptr<json_value>(move(properties)));

    if (required) schema.set_value("required", split(required, ','));
  }
};

template <typename value_t>
  requires described_json_object<value_t>
struct json_serializer<value_t>
{
  static json_object_descriptor<value_t>* get_type_description(const value_t& value)
  {
    auto id = type_index(typeid(value));
    auto result = &value_t::json_description;
    if (result->index != id)
    {
      result = result->derived_descriptors.at(id);
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
  requires is_pointing<value_t> && described_json_object<pointed_t<value_t>>
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

named_enum(test_enum, a, b, c);

struct test
{
  static json_object_descriptor<test> json_description;

  virtual ~test() = default;

  vector<test_enum> ve = { test_enum::a };
  float x = 1, y = 2, z = 3;
};

auto test::json_description = describe_json_object<test>("test", "some desc", {
    { &test::x, "x", {.description = "asd" } },
    { &test::y, "y" },
    { &test::z, "z", {.description = "asd", .minimum = 5 } },
    { &test::ve, "ve", {.description = "asd" } }
  });

struct test2 : public test
{
  bool b = true;

  static json_object_descriptor<test2> json_description;
};

auto test2::json_description = describe_json_object<test2, test>("test2", "some other desc", {
  { &test2::b, "b", {.description = "asd" } }
  });

struct test3 : public test2
{
  int c = 5;

  static json_object_descriptor<test3> json_description;
};

auto test3::json_description = describe_json_object<test3, test2>("test3", "deepest desc", {
  { &test3::c, "c", {.description = "asd" } }
  });

namespace Axodox::Common::Tests
{
  TEST_CLASS(ExperimentalTest)
  {
    TEST_METHOD(Test)
    {
      auto result = test3::json_description.to_json();

      auto o = make_value<json_object>();
      test x;
      auto y = &x;
      auto z = &y->ve;
      auto ww = &test::ve;
      OutputDebugStringA(format("main y{} z{}\n", (void*)y, (void*)z).c_str());
      test::json_description.to_json(x, o.get());

      auto text0 = result->to_string();
      auto text1 = stringify_json(test3{});
      auto text2 = stringify_json<value_ptr<test>>(make_value<test2>());
      auto value = try_parse_json<value_ptr<test>>(R"({"$type": "test2", "x": 6, "b": false, "ve": ["a"]})");
    }
  };
}