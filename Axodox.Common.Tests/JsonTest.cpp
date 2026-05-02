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

  auto animal::json_description = describe_json_object<animal>("animal", "any creature", {
    { &animal::name,        "name",         {.description = "given name", .pattern = "^[A-Za-z]+$"} },
    { &animal::age,         "age",          {.description = "in years",   .minimum = 0, .maximum = 200} },
    { &animal::is_friendly, "is_friendly",  {.description = "tame?"} },
    { &animal::mood,        "mood",         {.description = "current mood"} },
    { &animal::nicknames,   "nicknames",    {.description = "alternate names", .min_items = 0, .max_items = 5} },
  });

  struct dog : public animal
  {
    static json_object_descriptor<dog> json_description;

    int bark_volume = 5;
  };

  auto dog::json_description = describe_json_object<dog, animal>("dog", "a dog", {
    { &dog::bark_volume, "bark_volume", {.description = "0-10", .minimum = 0, .maximum = 10} },
  });

  struct dalmatian_dog : public dog
  {
    static json_object_descriptor<dalmatian_dog> json_description;

    int spot_count = 100;
  };

  auto dalmatian_dog::json_description = describe_json_object<dalmatian_dog, dog>("dalmatian_dog", "a spotted dog", {
    { &dalmatian_dog::spot_count, "spot_count", {.minimum = 0} },
  });

  struct horse : public animal
  {
    static json_object_descriptor<horse> json_description;

    string coat_color = "brown";
  };

  auto horse::json_description = describe_json_object<horse, animal>("horse", "a horse", {
    { &horse::coat_color, "coat_color" },
  });

  struct arabian_horse : public horse
  {
    static json_object_descriptor<arabian_horse> json_description;

    int lineage_score = 50;
  };

  auto arabian_horse::json_description = describe_json_object<arabian_horse, horse>("arabian_horse", "a noble horse", {
    { &arabian_horse::lineage_score, "lineage_score", {.minimum = 0, .maximum = 100} },
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
      Assert::AreEqual(size_t(7), props.size()); // 5 from animal + 1 from dog + 1 own
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

      auto text = stringify_json(source);
      auto parsed = try_parse_json<dog>(text);

      Assert::IsTrue(parsed.has_value(), L"dog failed to parse back");
      Assert::AreEqual(source.name, parsed->name);
      Assert::AreEqual(source.age, parsed->age);
      Assert::AreEqual(source.bark_volume, parsed->bark_volume);
    }

    TEST_METHOD(TestRoundTripDirectDalmatian)
    {
      dalmatian_dog source;
      source.name = "Pongo";
      source.bark_volume = 6;
      source.spot_count = 101;

      auto text = stringify_json(source);
      auto parsed = try_parse_json<dalmatian_dog>(text);

      Assert::IsTrue(parsed.has_value(), L"dalmatian failed to parse back");
      Assert::AreEqual(source.name, parsed->name);
      Assert::AreEqual(source.bark_volume, parsed->bark_volume);
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

      auto text = stringify_json(source);
      auto parsed = try_parse_json<value_ptr<animal>>(text);

      Assert::IsTrue(parsed.has_value(), L"polymorphic dog failed to parse");
      Assert::IsNotNull(parsed->get(), L"parsed pointer is null");

      auto* parsed_dog = dynamic_cast<dog*>(parsed->get());
      Assert::IsNotNull(parsed_dog, L"parsed value is not a dog");
      Assert::AreEqual<string>("Rex", parsed_dog->name);
      Assert::AreEqual(4, parsed_dog->age);
      Assert::AreEqual(7, parsed_dog->bark_volume);
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

      auto* bark_schema = as_object(properties->at("bark_volume"));
      Assert::AreEqual<string>("number", bark_schema->get_value<string>("type"));
      Assert::AreEqual(0.0, bark_schema->get_value<double>("minimum"));
      Assert::AreEqual(10.0, bark_schema->get_value<double>("maximum"));
    }
  };
}
