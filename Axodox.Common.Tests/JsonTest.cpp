#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.Json.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Json;
using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  named_enum(animal_mood, calm, excited, grumpy);

  struct animal
  {
    static json_object_descriptor<animal> json_description;

    virtual ~animal() = default;

    string name;
    int age = 0;
    bool is_friendly = true;
    animal_mood mood = animal_mood::calm;
    vector<string> nicknames;
  };

  json_object_descriptor<animal> animal::json_description = describe_json_object<animal>("animal", "any creature", {
    { &animal::name,        "name",         {.description = "given name", .pattern = "^[A-Za-z]+$"} },
    { &animal::age,         "age",          {.description = "in years",   .minimum = 0, .maximum = 200} },
    { &animal::is_friendly, "is_friendly",  {.description = "tame?"} },
    { &animal::mood,        "mood",         {.description = "current mood"} },
    { &animal::nicknames,   "nicknames",    {.description = "alternate names", .min_items = 0, .max_items = 5} },
    });

  enum class color { brown, white, black };

  struct dog_training
  {
    static json_object_descriptor<dog_training> json_description;

    bool sit = false;
    bool paw = false;
  };

  json_object_descriptor<dog_training> dog_training::json_description = describe_json_object<dog_training>({
    { &dog_training::sit, "sit" },
    { &dog_training::paw, "paw" }
    });

  static_assert(std::same_as<json_schema_type<std::optional<std::string>>, json_schema_type<std::string>>);
  static_assert(std::same_as<json_schema_type<std::optional<dog_training>>, json_schema_type<dog_training>>);

  struct dog : public animal
  {
    static json_object_descriptor<dog> json_description;

    int bark_volume = 5;
    color fur_color = color::brown;
    std::optional<dog_training> training;
  };

  json_object_descriptor<dog> dog::json_description = describe_json_object<dog, animal>("dog", "a dog", {
    { &dog::bark_volume, "bark_volume", {.description = "0-10", .minimum = 0, .maximum = 10} },
    { &dog::fur_color, "fur_color", {.description = "Brown, white or black." }},
    { &dog::training, "training" }
    });

  struct dalmatian_dog : public dog
  {
    static json_object_descriptor<dalmatian_dog> json_description;

    int spot_count = 100;
  };

  json_object_descriptor<dalmatian_dog> dalmatian_dog::json_description = describe_json_object<dalmatian_dog, dog>("dalmatian_dog", "a spotted dog", {
    { &dalmatian_dog::spot_count, "spot_count", {.minimum = 0} },
    });

  struct horse : public animal
  {
    static json_object_descriptor<horse> json_description;

    string coat_color = "brown";
  };

  json_object_descriptor<horse> horse::json_description = describe_json_object<horse, animal>("horse", "a horse", {
    { &horse::coat_color, "coat_color" },
    });

  struct arabian_horse : public horse
  {
    static json_object_descriptor<arabian_horse> json_description;

    int lineage_score = 50;
  };

  json_object_descriptor<arabian_horse> arabian_horse::json_description = describe_json_object<arabian_horse, horse>("arabian_horse", "a noble horse", {
    { &arabian_horse::lineage_score, "lineage_score", {.minimum = 0, .maximum = 100} },
    });

  // Parallel hierarchy that uses a custom "type" discriminator instead of the default "$type".
  struct vehicle
  {
    static json_object_descriptor<vehicle> json_description;

    virtual ~vehicle() = default;

    string model;
    int wheels = 4;
  };

  json_object_descriptor<vehicle> vehicle::json_description = describe_json_object<vehicle>(
    json_object_options{ .name = "vehicle", .description = "any vehicle", .type_discriminator = "type" },
    {
      { &vehicle::model,  "model" },
      { &vehicle::wheels, "wheels", {.minimum = 0} },
    });

    struct car : public vehicle
    {
      static json_object_descriptor<car> json_description;

      int seats = 5;
    };

    json_object_descriptor<car> car::json_description = describe_json_object<car, vehicle>(
      json_object_options{ .name = "car", .description = "a passenger car", .type_discriminator = "type" },
    {
      { &car::seats, "seats", {.minimum = 1} },
    });

    struct motorcycle : public vehicle
    {
      static json_object_descriptor<motorcycle> json_description;

      bool has_sidecar = false;
    };

    json_object_descriptor<motorcycle> motorcycle::json_description = describe_json_object<motorcycle, vehicle>(
      json_object_options{ .name = "motorcycle", .description = "a two-wheeler", .type_discriminator = "type" },
    {
      { &motorcycle::has_sidecar, "has_sidecar" },
    });

    enum class numeric_enum { a, b, c };

    struct typeless_test_object
    {
      static json_object_descriptor<typeless_test_object> json_description;

      optional<json_object> optional_object;
      value_ptr<json_value> any_value;
      json_array some_array;
    };

    json_object_descriptor<typeless_test_object> typeless_test_object::json_description = describe_json_object<typeless_test_object>({
      { &typeless_test_object::optional_object, "optional_object", {.description = "An optional object.", .required = "some_prop"}},
      { &typeless_test_object::any_value,       "any_value",       {.description = "Any value." }},
      { &typeless_test_object::some_array,      "some_array",      {.description = "An array.", .max_items = 10 }}
      });

    json_object* as_object(const value_ptr<json_value>& v)
    {
      Assert::IsNotNull(v.get(), L"json value is null");
      Assert::AreEqual(int(json_type::object), int(v->type()), L"json value is not an object");
      return static_cast<json_object*>(v.get());
    }
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(JsonSerializationTests)
  {
  public:
    TEST_METHOD(TestDescriptorRegistration)
    {
      Assert::AreEqual("animal", animal::json_description.name());
      Assert::AreEqual("dog", dog::json_description.name());
      Assert::AreEqual("dalmatian_dog", dalmatian_dog::json_description.name());
      Assert::AreEqual("horse", horse::json_description.name());
      Assert::AreEqual("arabian_horse", arabian_horse::json_description.name());

      // animal sees every descendant.
      auto& animal_derived = animal::json_description.derived_descriptors();
      Assert::IsTrue(animal_derived.contains(type_index(typeid(dog))));
      Assert::IsTrue(animal_derived.contains(type_index(typeid(dalmatian_dog))));
      Assert::IsTrue(animal_derived.contains(type_index(typeid(horse))));
      Assert::IsTrue(animal_derived.contains(type_index(typeid(arabian_horse))));

      // dog sees only its own subtree.
      auto& dog_derived = dog::json_description.derived_descriptors();
      Assert::IsTrue(dog_derived.contains(type_index(typeid(dalmatian_dog))));
      Assert::IsFalse(dog_derived.contains(type_index(typeid(horse))));
      Assert::IsFalse(dog_derived.contains(type_index(typeid(arabian_horse))));

      // dalmatian_dog inherits all properties from dog and animal.
      auto& props = dalmatian_dog::json_description.properties();
      Assert::AreEqual(size_t(9), props.size()); // 5 from animal + 3 from dog + 1 own
    }

    TEST_METHOD(TestRoundTripDirectAnimal)
    {
      animal source;
      source.name = "Felix";
      source.age = 7;
      source.is_friendly = false;
      source.mood = animal_mood::grumpy;
      source.nicknames = { "Fluffy", "Mr. Cat" };

      auto text = stringify_json(source);
      auto parsed = try_parse_json<animal>(text);

      Assert::IsTrue(parsed.has_value(), L"animal failed to parse back");
      Assert::AreEqual(source.name, parsed->name);
      Assert::AreEqual(source.age, parsed->age);
      Assert::AreEqual(source.is_friendly, parsed->is_friendly);
      Assert::IsTrue(source.mood == parsed->mood);
      Assert::AreEqual(source.nicknames.size(), parsed->nicknames.size());
      Assert::AreEqual(source.nicknames[0], parsed->nicknames[0]);
      Assert::AreEqual(source.nicknames[1], parsed->nicknames[1]);
    }

    TEST_METHOD(TestRoundTripDirectDog)
    {
      dog source;
      source.name = "Rex";
      source.age = 3;
      source.bark_volume = 8;
      source.fur_color = color::white;
      source.training = dog_training{ .sit = true, .paw = false };

      auto text = stringify_json(source);
      auto parsed = try_parse_json<dog>(text);

      Assert::IsTrue(parsed.has_value(), L"dog failed to parse back");
      Assert::AreEqual(source.name, parsed->name);
      Assert::AreEqual(source.age, parsed->age);
      Assert::AreEqual(source.bark_volume, parsed->bark_volume);
      Assert::IsTrue(source.fur_color == parsed->fur_color);
      Assert::IsTrue(parsed->training.has_value(), L"training optional was dropped");
      Assert::AreEqual(source.training->sit, parsed->training->sit);
      Assert::AreEqual(source.training->paw, parsed->training->paw);
    }

    TEST_METHOD(TestRoundTripDirectDogWithoutTraining)
    {
      dog source;
      source.name = "Buddy";
      source.age = 2;
      // training left as std::nullopt

      auto text = stringify_json(source);
      auto parsed = try_parse_json<dog>(text);

      Assert::IsTrue(parsed.has_value(), L"dog failed to parse back");
      Assert::IsFalse(parsed->training.has_value(), L"empty optional should round-trip as nullopt");
    }

    TEST_METHOD(TestRoundTripDirectDalmatian)
    {
      dalmatian_dog source;
      source.name = "Pongo";
      source.bark_volume = 6;
      source.fur_color = color::black;
      source.spot_count = 101;

      auto text = stringify_json(source);
      auto parsed = try_parse_json<dalmatian_dog>(text);

      Assert::IsTrue(parsed.has_value(), L"dalmatian failed to parse back");
      Assert::AreEqual(source.name, parsed->name);
      Assert::AreEqual(source.bark_volume, parsed->bark_volume);
      Assert::IsTrue(source.fur_color == parsed->fur_color);
      Assert::AreEqual(source.spot_count, parsed->spot_count);
    }

    TEST_METHOD(TestPolymorphicSerializeIncludesType)
    {
      value_ptr<animal> source = make_value<dog>();
      source->name = "Rex";
      static_cast<dog&>(*source).bark_volume = 9;

      auto json = json_serializer<value_ptr<animal>>::to_json(source);
      auto* obj = as_object(json);

      string type;
      Assert::IsTrue(obj->try_get_value<string>("$type", type), L"$type missing");
      Assert::AreEqual<string>("dog", type);
    }

    TEST_METHOD(TestPolymorphicRoundTripDogAsAnimal)
    {
      value_ptr<animal> source = make_value<dog>();
      source->name = "Rex";
      source->age = 4;
      static_cast<dog&>(*source).bark_volume = 7;
      static_cast<dog&>(*source).fur_color = color::black;

      auto text = stringify_json(source);
      auto parsed = try_parse_json<value_ptr<animal>>(text);

      Assert::IsTrue(parsed.has_value(), L"polymorphic dog failed to parse");
      Assert::IsNotNull(parsed->get(), L"parsed pointer is null");

      auto* parsed_dog = dynamic_cast<dog*>(parsed->get());
      Assert::IsNotNull(parsed_dog, L"parsed value is not a dog");
      Assert::AreEqual<string>("Rex", parsed_dog->name);
      Assert::AreEqual(4, parsed_dog->age);
      Assert::AreEqual(7, parsed_dog->bark_volume);
      Assert::IsTrue(color::black == parsed_dog->fur_color);
    }

    TEST_METHOD(TestPolymorphicRoundTripDalmatianAsAnimal)
    {
      value_ptr<animal> source = make_value<dalmatian_dog>();
      source->name = "Pongo";
      static_cast<dalmatian_dog&>(*source).spot_count = 99;

      auto text = stringify_json(source);
      auto parsed = try_parse_json<value_ptr<animal>>(text);

      Assert::IsTrue(parsed.has_value());
      auto* spotted = dynamic_cast<dalmatian_dog*>(parsed->get());
      Assert::IsNotNull(spotted, L"parsed value is not a dalmatian");
      Assert::AreEqual<string>("Pongo", spotted->name);
      Assert::AreEqual(99, spotted->spot_count);
    }

    TEST_METHOD(TestPolymorphicRejectsSiblingType)
    {
      // A dog payload aimed at value_ptr<horse>: must reject.
      value_ptr<animal> source_dog = make_value<dog>();
      source_dog->name = "Rex";
      auto dog_text = stringify_json(source_dog);

      auto parsed = try_parse_json<value_ptr<horse>>(dog_text);
      Assert::IsFalse(parsed.has_value(), L"horse pointer accepted a dog payload");
    }

    TEST_METHOD(TestPolymorphicRejectsCousinType)
    {
      // An arabian_horse payload aimed at value_ptr<dog>: must reject.
      value_ptr<animal> source_horse = make_value<arabian_horse>();
      source_horse->name = "Bucephalus";
      auto horse_text = stringify_json(source_horse);

      auto parsed = try_parse_json<value_ptr<dog>>(horse_text);
      Assert::IsFalse(parsed.has_value(), L"dog pointer accepted an arabian_horse payload");
    }

    TEST_METHOD(TestPolymorphicAcceptsDescendantOfStaticType)
    {
      // A dalmatian_dog payload aimed at value_ptr<dog>: must produce a dalmatian_dog.
      value_ptr<animal> source = make_value<dalmatian_dog>();
      source->name = "Pongo";
      static_cast<dalmatian_dog&>(*source).spot_count = 102;
      auto text = stringify_json(source);

      auto parsed = try_parse_json<value_ptr<dog>>(text);
      Assert::IsTrue(parsed.has_value(), L"value_ptr<dog> rejected a dalmatian payload");
      auto* spotted = dynamic_cast<dalmatian_dog*>(parsed->get());
      Assert::IsNotNull(spotted, L"value_ptr<dog> did not produce a dalmatian");
      Assert::AreEqual(102, spotted->spot_count);
    }

    TEST_METHOD(TestSchemaForAnimal)
    {
      auto schema = animal::json_description.to_json();
      auto* root = as_object(schema);

      // Root is an object schema.
      Assert::AreEqual<string>("object", root->get_value<string>("type"));

      // Properties present.
      auto* properties = as_object(root->at("properties"));
      Assert::AreEqual(size_t(5), properties->value.size());

      // String property: name (with pattern).
      auto* name_schema = as_object(properties->at("name"));
      Assert::AreEqual<string>("string", name_schema->get_value<string>("type"));
      Assert::AreEqual<string>("^[A-Za-z]+$", name_schema->get_value<string>("pattern"));

      // Number property: age (with bounds).
      auto* age_schema = as_object(properties->at("age"));
      Assert::AreEqual<string>("number", age_schema->get_value<string>("type"));
      Assert::AreEqual(0.0, age_schema->get_value<double>("minimum"));
      Assert::AreEqual(200.0, age_schema->get_value<double>("maximum"));

      // Boolean property.
      auto* friendly_schema = as_object(properties->at("is_friendly"));
      Assert::AreEqual<string>("boolean", friendly_schema->get_value<string>("type"));

      // Enum property: mood (named_enum -> string with "enum" array).
      auto* mood_schema = as_object(properties->at("mood"));
      Assert::AreEqual<string>("string", mood_schema->get_value<string>("type"));
      auto enum_values = mood_schema->get_value<vector<string>>("enum");
      Assert::AreEqual(size_t(3), enum_values.size());
      Assert::IsTrue(ranges::find(enum_values, "calm") != enum_values.end());
      Assert::IsTrue(ranges::find(enum_values, "excited") != enum_values.end());
      Assert::IsTrue(ranges::find(enum_values, "grumpy") != enum_values.end());

      // Array property: nicknames (with item type and bounds).
      auto* nicknames_schema = as_object(properties->at("nicknames"));
      Assert::AreEqual<string>("array", nicknames_schema->get_value<string>("type"));
      Assert::AreEqual(0.0, nicknames_schema->get_value<double>("minItems"));
      Assert::AreEqual(5.0, nicknames_schema->get_value<double>("maxItems"));
      auto* items_schema = as_object(nicknames_schema->at("items"));
      Assert::AreEqual<string>("string", items_schema->get_value<string>("type"));
    }

    TEST_METHOD(TestSchemaForDogIncludesInheritedProperties)
    {
      auto schema = dog::json_description.to_json();
      auto* root = as_object(schema);
      auto* properties = as_object(root->at("properties"));

      // Inherited from animal:
      Assert::IsTrue(properties->value.contains("name"));
      Assert::IsTrue(properties->value.contains("age"));
      Assert::IsTrue(properties->value.contains("mood"));
      // Dog's own:
      Assert::IsTrue(properties->value.contains("bark_volume"));
      Assert::IsTrue(properties->value.contains("fur_color"));
      Assert::IsTrue(properties->value.contains("training"));

      auto* bark_schema = as_object(properties->at("bark_volume"));
      Assert::AreEqual<string>("number", bark_schema->get_value<string>("type"));
      Assert::AreEqual(0.0, bark_schema->get_value<double>("minimum"));
      Assert::AreEqual(10.0, bark_schema->get_value<double>("maximum"));

      // fur_color is a numeric enum class, so it serializes as a number.
      auto* fur_schema = as_object(properties->at("fur_color"));
      Assert::AreEqual<string>("number", fur_schema->get_value<string>("type"));

      // training is std::optional<dog_training>: it surfaces as the underlying described object.
      auto* training_schema = as_object(properties->at("training"));
      Assert::AreEqual<string>("object", training_schema->get_value<string>("type"));
      auto* training_props = as_object(training_schema->at("properties"));
      Assert::IsTrue(training_props->value.contains("sit"));
      Assert::IsTrue(training_props->value.contains("paw"));
    }

    TEST_METHOD(TestCustomTypeDiscriminatorIsConfigured)
    {
      Assert::AreEqual<string>("type", vehicle::json_description.type_discriminator());
      Assert::AreEqual<string>("type", car::json_description.type_discriminator());
      Assert::AreEqual<string>("type", motorcycle::json_description.type_discriminator());

      // Default discriminator on the unrelated hierarchy is unaffected.
      Assert::AreEqual<string>("$type", animal::json_description.type_discriminator());
    }

    TEST_METHOD(TestCustomTypeDiscriminatorSerializeUsesCustomKey)
    {
      value_ptr<vehicle> source = make_value<car>();
      source->model = "Roadster";
      static_cast<car&>(*source).seats = 2;

      auto json = json_serializer<value_ptr<vehicle>>::to_json(source);
      auto* obj = as_object(json);

      // The custom "type" key carries the discriminator; "$type" must not be emitted.
      string type;
      Assert::IsTrue(obj->try_get_value<string>("type", type), L"custom 'type' discriminator missing");
      Assert::AreEqual<string>("car", type);

      json_value* dollar_type;
      Assert::IsFalse(obj->try_get_value("$type", dollar_type), L"unexpected '$type' present");
    }

    TEST_METHOD(TestCustomTypeDiscriminatorPolymorphicRoundTrip)
    {
      value_ptr<vehicle> source = make_value<motorcycle>();
      source->model = "Scrambler";
      source->wheels = 2;
      static_cast<motorcycle&>(*source).has_sidecar = true;

      auto text = stringify_json(source);

      // Sanity check: serialized text uses "type", not "$type".
      Assert::IsTrue(text.find("\"type\"") != string::npos, L"serialized payload missing custom 'type' key");
      Assert::IsTrue(text.find("\"$type\"") == string::npos, L"serialized payload contains '$type'");

      auto parsed = try_parse_json<value_ptr<vehicle>>(text);
      Assert::IsTrue(parsed.has_value(), L"polymorphic motorcycle failed to parse");

      auto* parsed_motorcycle = dynamic_cast<motorcycle*>(parsed->get());
      Assert::IsNotNull(parsed_motorcycle, L"parsed value is not a motorcycle");
      Assert::AreEqual<string>("Scrambler", parsed_motorcycle->model);
      Assert::AreEqual(2, parsed_motorcycle->wheels);
      Assert::IsTrue(parsed_motorcycle->has_sidecar);
    }

    TEST_METHOD(TestCustomTypeDiscriminatorIgnoresDollarType)
    {
      // A payload that uses the legacy "$type" key must not be honored when the descriptor
      // declares a different discriminator: the parse should fall back to the static type.
      auto text = string{ R"({"$type":"motorcycle","model":"Imposter","wheels":4,"seats":3})" };

      auto parsed = try_parse_json<value_ptr<vehicle>>(text);
      Assert::IsTrue(parsed.has_value(), L"vehicle payload failed to parse");

      // No discriminator under the configured key -> we get the base type, not the motorcycle.
      Assert::IsNull(dynamic_cast<motorcycle*>(parsed->get()), L"'$type' was honored despite custom discriminator");
      Assert::AreEqual<string>("Imposter", (*parsed)->model);
    }

    TEST_METHOD(TestNumericEnumSerialization)
    {
      auto json = stringify_json(numeric_enum::b);
      auto value = try_parse_json<numeric_enum>(json);
      Assert::IsTrue(value.has_value());
      Assert::IsTrue(*value == numeric_enum::b);
    }

    template<typename value_t = value_ptr<json_value>>
    void TestSerialization(std::string_view text)
    {
      auto value = try_parse_json<value_t>(text);
      auto json = stringify_json(value);
      Assert::AreEqual<std::string_view>(text, json);
    }

    TEST_METHOD(TestTypelessValueSerialization)
    {
      TestSerialization("1.14");
      TestSerialization("true");
      TestSerialization("\"asd\"");
      TestSerialization("[\"asd\",\"qwe\"]");
      TestSerialization("{\"list\":[\"asd\",\"qwe\"]}");
    }

    TEST_METHOD(TestTypelessArraySerialization)
    {
      TestSerialization<optional<json_array>>("[\"asd\",\"qwe\"]");
      TestSerialization<json_array>("[\"asd\",\"qwe\"]");
    }

    TEST_METHOD(TestTypelessObjectSerialization)
    {
      TestSerialization<optional<json_object>>("{\"list\":[\"asd\",\"qwe\"]}");
      TestSerialization<json_object>("{\"list\":[\"asd\",\"qwe\"]}");
    }

    TEST_METHOD(TestTypelessSchema)
    {
      TestSerialization<typeless_test_object>("{\"optional_object\":{\"a\":5.6,\"b\":true},\"any_value\":1.2,\"some_array\":[1,2,3,false,\"asd\"]}");

      auto schema = typeless_test_object::json_description.to_json()->to_string();
      Assert::AreEqual<string_view>("{\"type\":\"object\",\"properties\":{\"optional_object\":{\"type\":\"object\",\"description\":\"An optional object.\",\"required\":[\"some_prop\"]},\"any_value\":{\"description\":\"Any value.\"},\"some_array\":{\"type\":\"array\",\"description\":\"An array.\",\"maxItems\":10,\"items\":{}}}}", schema);
    }
  };
}
