#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.Json.h"

using namespace Axodox::Json;
using namespace Axodox::Infrastructure;
using namespace std;

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

template<typename value_t, json_type type_v>
struct json_schema_base
{
  json_type type = type_v;
  schema_generator filler = [](const json_schema_variant* that, json_object& schema) {
    reinterpret_cast<const value_t*>(that)->populate_schema(schema);
    };

  void populate_schema(json_object& schema) const
  {
    //Do nothing
  }

  value_ptr<json_value> to_json() const
  {
    return reinterpret_cast<const json_schema_variant*>(this)->to_json();
  }
};

template<typename value_t>
struct value_optional
{
  bool has_value;
  value_t value;

  constexpr value_optional() :
    has_value(false),
    value(value_t{})
  {
  }

  constexpr value_optional(value_t value) :
    has_value(true),
    value(value)
  {
  }

  explicit operator bool() const
  {
    return has_value;
  }

  value_t operator*() const
  {
    return value;
  }
};

struct json_number_schema : public json_schema_base<json_number_schema, json_type::number>
{
  const char* description = nullptr;
  value_optional<double> minimum;
  value_optional<double> maximum;

  void populate_schema(json_object& schema) const
  {
    if (description) schema.set_value("description", description);
    if (minimum) schema.set_value("minimum", *minimum);
    if (maximum) schema.set_value("maximum", *maximum);
  }
};

struct json_boolean_schema : public json_schema_base<json_boolean_schema, json_type::boolean>
{
  const char* description = nullptr;

  void populate_schema(json_object& schema) const
  {
    if (description) schema.set_value("description", description);
  }
};

struct json_string_schema : public json_schema_base<json_string_schema, json_type::string>
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

template<typename value_t>
struct json_schema
{
  using type = void;
};

template<typename value_t>
using json_schema_type = json_schema<value_t>::type;

template<typename item_t>
struct json_array_schema : public json_schema_base<json_array_schema<item_t>, json_type::array>
{
  const char* description = nullptr;
  value_optional<int32_t> min_items;
  value_optional<int32_t> max_items;
  json_schema_type<item_t> items;

  void populate_schema(json_object& schema) const
  {
    if (description) schema.set_value("description", description);
    if (min_items) schema.set_value("minItems", *min_items);
    if (max_items) schema.set_value("maxItems", *max_items);

    schema.set_value("items", items.to_json());
  }
};

template<typename value_t, json_schema_type<value_t> schema_v = {}, typename converter_t = json_serializer<value_t >>
  struct json_schema_property : public json_property<value_t, converter_t>
{
  const json_schema_variant* schema;

  json_schema_property(json_object_base* owner, const char* name, value_t value = value_t{}) :
    json_property<value_t>(owner, name, value),
    schema(reinterpret_cast<const json_schema_variant*>(&schema_v)) //we will make this safe by making the schema a literal value in the code
  {
  }
};

template<typename object_t>
struct json_object_schema : public json_schema_base<json_object_schema<object_t>, json_type::object>
{
  const char* description = nullptr;
  const char* required = nullptr;

  void populate_schema(json_object& schema) const
  {
    if (description) schema.set_value("description", description);
    if (required) schema.set_value("required", split(required, ','));

    auto propertySchema = make_value<json_object>();

    object_t instance;
    for (auto property : instance.properties())
    {
      auto schemaOffset = property->size();
      auto schemaAlignment = alignof(const json_schema_variant*);
      if (schemaOffset % schemaAlignment != 0) schemaOffset += schemaAlignment - schemaOffset % schemaAlignment;

      auto schemaProperty = *reinterpret_cast<const json_schema_variant**>(intptr_t(property) + schemaOffset);
      propertySchema->set_value(property->name(), schemaProperty->to_json());
    }
    schema.set_value("properties", value_ptr<json_value>(move(propertySchema)));
  }
};

template<>
struct json_schema<bool>
{
  using type = json_boolean_schema;
};

template<typename value_t>
  requires std::is_arithmetic_v<value_t>
struct json_schema<value_t>
{
  using type = json_number_schema;
};

template<>
struct json_schema<std::string>
{
  using type = json_string_schema;
};

template<typename value_t>
struct json_schema<std::vector<value_t>>
{
  using type = json_array_schema<value_t>;
};

template<std::derived_from<json_object_base> value_t>
struct json_schema<value_t>
{
  using type = json_object_schema<value_t>;
};

struct test_nested_object : public json_object_base
{
  json_schema_property<bool, {
    .description = "Some other bool"
  } > x{ this, "x" };
};

template<typename object_t, json_object_schema<object_t> schema_v>
struct json_schema_object_base : public json_object_base
{
  const json_object_schema<object_t>* schema = &schema_v;
};

struct test_object : public json_schema_object_base < test_object, {
  .description = "test"
} >
{
  json_schema_property<bool, {
    .description = "Some bool"
  } > x{ this, "x" };

  json_schema_property<float, {
    .description = "Some float",
    .minimum = 5
  } > y{ this, "y" };

  json_schema_property < string, {
    .description = "Some text",
    .pattern = ".*"
  } > z{ this, "z" };

  json_schema_property < std::vector<int>, {
    .description = "Some array",
    .max_items = 2,
    .items = {
      .minimum = 30
    }
  } > w{ this, "w" };

  json_schema_property < test_nested_object, {
    .description = "Some object",
    .required = "x,y"
  } > q{ this, "q" };
};

template<typename object_t>
struct json_property_descriptor
{
  template<typename value_t>
  using typed_field_ptr = value_t(object_t::*);

  ptrdiff_t field;
  const char* name;
  const void* schema;

  template<typename value_t>
  json_property_descriptor(typed_field_ptr<value_t> field, const char* name, json_schema_type<value_t>&& schema = {}) :
    field(ptrdiff_t(&(reinterpret_cast<object_t*>(nullptr)->*field))),
    name(name),
    schema(new json_schema_type<value_t>(move(schema)))
  {
  }
};

template<typename object_t>
auto describe(std::initializer_list<json_property_descriptor<object_t>> properties)
{
  return vector<json_property_descriptor<object_t>>{ properties.begin(), properties.end() };
}

struct test
{
  float x, y, z;

  inline static auto description = describe<test>({
    { &test::x, "x", { .description = "asd" } },
    { &test::y, "y" },
    { &test::z, "z", { .description = "asd", .minimum = 5 } },
  });
};

namespace Axodox::Common::Tests
{



  TEST_CLASS(ExperimentalTest)
  {
    TEST_METHOD(Test)
    {
      using type = json_schema_property<bool, {
        .description = "Some other bool 2"
      } > ;
      auto test = offsetof(type, schema);


      test_object t;
      auto value = t.schema->to_json();
      auto result = value->to_string();
    }
  };
}

