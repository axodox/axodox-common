#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.Storage.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Storage;
using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  struct creature
  {
    static binary_object_descriptor<creature> binary_description;

    virtual ~creature() = default;

    string name;
    int age = 0;
    bool is_friendly = true;
  };

  auto creature::binary_description = describe_binary_object<creature>(
    binary_object_options{ .type_id = 1, .version = 1 },
    {
      { &creature::name },
      { &creature::age },
      { &creature::is_friendly },
    });

  struct hound : public creature
  {
    static binary_object_descriptor<hound> binary_description;

    int bark_volume = 5;
  };

  auto hound::binary_description = describe_binary_object<hound, creature>(
    binary_object_options{ .type_id = 2, .version = 1 },
    {
      { &hound::bark_volume },
    });

  struct beagle : public hound
  {
    static binary_object_descriptor<beagle> binary_description;

    int spot_count = 12;
  };

  auto beagle::binary_description = describe_binary_object<beagle, hound>(
    binary_object_options{ .type_id = 3, .version = 1 },
    {
      { &beagle::spot_count },
    });

  struct stallion : public creature
  {
    static binary_object_descriptor<stallion> binary_description;

    int gallop_speed = 30;
  };

  auto stallion::binary_description = describe_binary_object<stallion, creature>(
    binary_object_options{ .type_id = 4, .version = 1 },
    {
      { &stallion::gallop_speed },
    });

  // Versioned record: tag was added at version 2, weight removed after version 1.
  // Serializing at version 2 should emit name + tag (no weight); deserializing a v1
  // payload should populate name + weight (no tag).
  struct package
  {
    static binary_object_descriptor<package> binary_description;

    string name;
    int weight = 0;       // present in v0..v1
    string tag;           // present from v2 onward

    static constexpr version_t v1 = 1;
    static constexpr version_t v2 = 2;
  };

  auto package::binary_description = describe_binary_object<package>(
    binary_object_options{ .type_id = 10, .version = package::v2 },
    {
      { &package::name },
      { &package::weight, binary_property_options{.max_version = package::v1 } },
      { &package::tag,    binary_property_options{.min_version = package::v2 } },
    });

  struct package_with_containers
  {
    static binary_object_descriptor<package_with_containers> binary_description;

    std::vector<std::string> tags;
    std::list<int> sequence;
    std::set<std::string> labels;
    std::map<std::string, int> counts;
  };

  auto package_with_containers::binary_description = describe_binary_object<package_with_containers>(
    binary_object_options{ .type_id = 100, .version = 1 },
    {
      { &package_with_containers::tags },
      { &package_with_containers::sequence },
      { &package_with_containers::labels },
      { &package_with_containers::counts },
    });

  // Custom converter for string properties that uppercases on write and lowercases on read.
  // Lets us prove via a round-trip that the descriptor really invoked our converter and not
  // the default binary_serializer<string>.
  struct uppercase_string_converter
  {
    void serialize(stream& s, const std::string& value) const
    {
      std::string upper(value);
      for (auto& c : upper) c = char(std::toupper(static_cast<unsigned char>(c)));
      s.write(upper);
    }

    void deserialize(stream& s, std::string& value) const
    {
      s.read(value);
      for (auto& c : value) c = char(std::tolower(static_cast<unsigned char>(c)));
    }
  };

  // Convertible to one type only — used to probe the negative side of the constraint.
  struct int_only_converter
  {
    void serialize(stream& s, const int& value) const { s.write(value); }
    void deserialize(stream& s, int& value) const { s.read(value); }
  };

  static_assert(binary_converter<binary_serializer<int>, int>,
    "default binary_serializer<int> must satisfy binary_converter<_, int>");
  static_assert(binary_converter<uppercase_string_converter, std::string>,
    "uppercase_string_converter must satisfy binary_converter<_, std::string>");
  static_assert(binary_converter<int_only_converter, int>,
    "int_only_converter must satisfy binary_converter<_, int>");
  static_assert(!binary_converter<int_only_converter, std::string>,
    "int_only_converter must NOT satisfy binary_converter<_, std::string>");
  static_assert(!binary_converter<uppercase_string_converter, int>,
    "uppercase_string_converter must NOT satisfy binary_converter<_, int>");

  // Constructibility probes: passing a converter whose value type does not match the field
  // type must be rejected by the constrained constructor of binary_property_descriptor.
  struct probe
  {
    int n;
    std::string s;
  };

  static_assert(std::constructible_from<binary_property_descriptor<probe>,
    decltype(&probe::s), binary_property_options, uppercase_string_converter>,
    "string converter must bind to a string field");
  static_assert(!std::constructible_from<binary_property_descriptor<probe>,
    decltype(&probe::n), binary_property_options, uppercase_string_converter>,
    "string converter must NOT bind to an int field");
  static_assert(!std::constructible_from<binary_property_descriptor<probe>,
    decltype(&probe::s), binary_property_options, int_only_converter>,
    "int converter must NOT bind to a string field");

  // Described type that uses the custom converter on its 'tag' property.
  struct shouting_message
  {
    static binary_object_descriptor<shouting_message> binary_description;

    std::string tag;
    int count = 0;
  };

  auto shouting_message::binary_description = describe_binary_object<shouting_message>(
    binary_object_options{ .type_id = 200, .version = 1 },
    {
      { &shouting_message::tag, {}, uppercase_string_converter{} },
      { &shouting_message::count },
    });
}

namespace Axodox::Common::Tests
{
  TEST_CLASS(BinarySerializationTests)
  {
  public:
    TEST_METHOD(TestDescriptorRegistration)
    {
      Assert::AreEqual<type_id_t>(1, creature::binary_description.id());
      Assert::AreEqual<type_id_t>(2, hound::binary_description.id());
      Assert::AreEqual<type_id_t>(3, beagle::binary_description.id());
      Assert::AreEqual<type_id_t>(4, stallion::binary_description.id());

      Assert::AreEqual<version_t>(1, creature::binary_description.version());

      // creature sees every descendant.
      auto& creature_derived = creature::binary_description.derived_descriptors();
      Assert::IsTrue(creature_derived.contains(type_index(typeid(hound))));
      Assert::IsTrue(creature_derived.contains(type_index(typeid(beagle))));
      Assert::IsTrue(creature_derived.contains(type_index(typeid(stallion))));

      // hound only sees its own subtree.
      auto& hound_derived = hound::binary_description.derived_descriptors();
      Assert::IsTrue(hound_derived.contains(type_index(typeid(beagle))));
      Assert::IsFalse(hound_derived.contains(type_index(typeid(stallion))));

      // beagle inherits all properties from hound and creature.
      auto& props = beagle::binary_description.properties();
      Assert::AreEqual(size_t(5), props.size()); // 3 from creature + 1 from hound + 1 own
    }

    TEST_METHOD(TestRoundTripDirectCreature)
    {
      creature source;
      source.name = "Felix";
      source.age = 7;
      source.is_friendly = false;

      auto bytes = to_bytes(source);
      auto parsed = from_bytes<creature>(bytes);

      Assert::AreEqual(source.name, parsed.name);
      Assert::AreEqual(source.age, parsed.age);
      Assert::AreEqual(source.is_friendly, parsed.is_friendly);
    }

    TEST_METHOD(TestRoundTripDirectHound)
    {
      hound source;
      source.name = "Rex";
      source.age = 3;
      source.bark_volume = 8;

      auto bytes = to_bytes(source);
      auto parsed = from_bytes<hound>(bytes);

      Assert::AreEqual(source.name, parsed.name);
      Assert::AreEqual(source.age, parsed.age);
      Assert::AreEqual(source.bark_volume, parsed.bark_volume);
    }

    TEST_METHOD(TestRoundTripDirectBeagle)
    {
      beagle source;
      source.name = "Snoopy";
      source.bark_volume = 6;
      source.spot_count = 9;

      auto bytes = to_bytes(source);
      auto parsed = from_bytes<beagle>(bytes);

      Assert::AreEqual(source.name, parsed.name);
      Assert::AreEqual(source.bark_volume, parsed.bark_volume);
      Assert::AreEqual(source.spot_count, parsed.spot_count);
    }

    TEST_METHOD(TestPolymorphicSerializeIncludesTypeId)
    {
      value_ptr<creature> source = make_value<hound>();
      source->name = "Rex";
      static_cast<hound&>(*source).bark_volume = 9;

      auto bytes = to_bytes(source);

      // Wire format: [type_id_t (id of dynamic type)][version_t][properties...].
      // First two bytes must be hound's id.
      Assert::IsTrue(bytes.size() >= sizeof(type_id_t), L"payload too short");
      type_id_t encoded_id;
      memcpy(&encoded_id, bytes.data(), sizeof(type_id_t));
      Assert::AreEqual<type_id_t>(hound::binary_description.id(), encoded_id);
    }

    TEST_METHOD(TestPolymorphicRoundTripHoundAsCreature)
    {
      value_ptr<creature> source = make_value<hound>();
      source->name = "Rex";
      source->age = 4;
      static_cast<hound&>(*source).bark_volume = 7;

      auto bytes = to_bytes(source);
      auto parsed = from_bytes<value_ptr<creature>>(bytes);

      Assert::IsNotNull(parsed.get(), L"parsed pointer is null");
      auto* parsed_hound = dynamic_cast<hound*>(parsed.get());
      Assert::IsNotNull(parsed_hound, L"parsed value is not a hound");
      Assert::AreEqual<string>("Rex", parsed_hound->name);
      Assert::AreEqual(4, parsed_hound->age);
      Assert::AreEqual(7, parsed_hound->bark_volume);
    }

    TEST_METHOD(TestPolymorphicRoundTripBeagleAsCreature)
    {
      value_ptr<creature> source = make_value<beagle>();
      source->name = "Snoopy";
      static_cast<beagle&>(*source).spot_count = 11;

      auto bytes = to_bytes(source);
      auto parsed = from_bytes<value_ptr<creature>>(bytes);

      auto* spotted = dynamic_cast<beagle*>(parsed.get());
      Assert::IsNotNull(spotted, L"parsed value is not a beagle");
      Assert::AreEqual<string>("Snoopy", spotted->name);
      Assert::AreEqual(11, spotted->spot_count);
    }

    TEST_METHOD(TestPolymorphicAcceptsDescendantOfStaticType)
    {
      // A beagle payload aimed at value_ptr<hound> should produce a beagle.
      value_ptr<creature> source = make_value<beagle>();
      source->name = "Snoopy";
      static_cast<beagle&>(*source).spot_count = 14;

      auto bytes = to_bytes(source);
      auto parsed = from_bytes<value_ptr<hound>>(bytes);

      auto* spotted = dynamic_cast<beagle*>(parsed.get());
      Assert::IsNotNull(spotted, L"value_ptr<hound> did not produce a beagle");
      Assert::AreEqual(14, spotted->spot_count);
    }

    TEST_METHOD(TestPolymorphicRejectsCousinType)
    {
      // A stallion payload deserialized as value_ptr<hound> must throw,
      // because stallion's id is not registered under hound's derived set.
      value_ptr<creature> source = make_value<stallion>();
      source->name = "Bucephalus";
      auto bytes = to_bytes(source);

      Assert::ExpectException<std::runtime_error>(
        [&] { from_bytes<value_ptr<hound>>(bytes); },
        L"value_ptr<hound> accepted a stallion payload");
    }

    TEST_METHOD(TestPolymorphicNullRoundTrip)
    {
      value_ptr<creature> source; // null
      auto bytes = to_bytes(source);

      auto parsed = from_bytes<value_ptr<creature>>(bytes);
      Assert::IsNull(parsed.get(), L"null pointer round-trip produced a non-null value");
    }

    TEST_METHOD(TestVersionedSerializeOmitsRetiredProperty)
    {
      // package's descriptor is at v2, so 'weight' (max_version = v1) must NOT be emitted,
      // and 'tag' (min_version = v2) MUST be emitted.
      package source;
      source.name = "Box";
      source.weight = 999;       // should not appear on the wire
      source.tag = "fragile";

      auto bytes = to_bytes(source);
      auto parsed = from_bytes<package>(bytes);

      Assert::AreEqual<string>("Box", parsed.name);
      Assert::AreEqual<string>("fragile", parsed.tag);
      // weight was not emitted, so it stays at the default-constructed value on read.
      Assert::AreEqual(0, parsed.weight);
    }

    TEST_METHOD(TestVersionedReadsLegacyPayload)
    {
      // Hand-craft a v1 payload: [version=1][name][weight].
      // 'tag' did not exist at v1, so it must not be read.
      memory_stream stream;
      stream.write(version_t(package::v1));
      stream.write(string("LegacyBox"));
      stream.write(int(42));

      array_stream input{ std::span<const uint8_t>(stream) };
      auto parsed = from_stream<package>(input);

      Assert::AreEqual<string>("LegacyBox", parsed.name);
      Assert::AreEqual(42, parsed.weight);
      Assert::AreEqual<string>("", parsed.tag);
    }
  };

  // Round-trips for the container specializations of binary_serializer. These exercise the
  // serializer directly (rather than through a descriptor) since each test only cares about
  // a single field type.
  template <typename container_t>
  static container_t roundtrip(const container_t& source)
  {
    memory_stream out;
    binary_serializer<container_t> serializer;
    serializer.serialize(out, source);

    array_stream in{ std::span<const uint8_t>(out) };
    container_t parsed;
    serializer.deserialize(in, parsed);
    return parsed;
  }

  TEST_CLASS(BinaryContainerSerializerTests)
  {
  public:
    TEST_METHOD(TestVectorOfStrings)
    {
      vector<string> source{ "alpha", "beta", "gamma" };
      auto parsed = roundtrip(source);

      Assert::AreEqual(source.size(), parsed.size());
      for (size_t i = 0; i < source.size(); i++)
      {
        Assert::AreEqual(source[i], parsed[i]);
      }
    }

    TEST_METHOD(TestVectorOfInts)
    {
      // Trivially-copyable element type: served by the primary template via the bulk
      // stream::write(vector<T>) fast path. Round-trip should still be lossless.
      vector<int> source{ 1, 2, 3, 4, 5 };
      auto parsed = roundtrip(source);

      Assert::AreEqual(source.size(), parsed.size());
      for (size_t i = 0; i < source.size(); i++)
      {
        Assert::AreEqual(source[i], parsed[i]);
      }
    }

    TEST_METHOD(TestEmptyVector)
    {
      vector<string> source;
      auto parsed = roundtrip(source);
      Assert::IsTrue(parsed.empty());
    }

    TEST_METHOD(TestList)
    {
      list<string> source{ "first", "second", "third" };
      auto parsed = roundtrip(source);

      Assert::AreEqual(source.size(), parsed.size());
      auto a = source.begin();
      auto b = parsed.begin();
      for (; a != source.end(); ++a, ++b)
      {
        Assert::AreEqual(*a, *b);
      }
    }

    TEST_METHOD(TestSet)
    {
      set<string> source{ "apple", "banana", "cherry" };
      auto parsed = roundtrip(source);

      Assert::AreEqual(source.size(), parsed.size());
      for (auto& item : source)
      {
        Assert::IsTrue(parsed.contains(item));
      }
    }

    TEST_METHOD(TestUnorderedSet)
    {
      unordered_set<int> source{ 10, 20, 30, 40 };
      auto parsed = roundtrip(source);

      Assert::AreEqual(source.size(), parsed.size());
      for (auto& item : source)
      {
        Assert::IsTrue(parsed.contains(item));
      }
    }

    TEST_METHOD(TestMap)
    {
      map<string, int> source{ {"one", 1}, {"two", 2}, {"three", 3} };
      auto parsed = roundtrip(source);

      Assert::AreEqual(source.size(), parsed.size());
      for (auto& [key, value] : source)
      {
        auto it = parsed.find(key);
        Assert::IsTrue(it != parsed.end(), L"key missing in parsed map");
        Assert::AreEqual(value, it->second);
      }
    }

    TEST_METHOD(TestUnorderedMap)
    {
      unordered_map<int, string> source{ {1, "one"}, {2, "two"}, {3, "three"} };
      auto parsed = roundtrip(source);

      Assert::AreEqual(source.size(), parsed.size());
      for (auto& [key, value] : source)
      {
        auto it = parsed.find(key);
        Assert::IsTrue(it != parsed.end(), L"key missing in parsed map");
        Assert::AreEqual(value, it->second);
      }
    }

    TEST_METHOD(TestEmptyMap)
    {
      map<string, int> source;
      auto parsed = roundtrip(source);
      Assert::IsTrue(parsed.empty());
    }

    TEST_METHOD(TestNestedVectorOfVectors)
    {
      // Nested non-trivial container: outer specialization writes the count then asks the
      // inner serializer to write each vector<int>, which uses the primary template's
      // bulk fast path.
      vector<vector<int>> source{ {1, 2, 3}, {}, {7, 8} };
      auto parsed = roundtrip(source);

      Assert::AreEqual(source.size(), parsed.size());
      for (size_t i = 0; i < source.size(); i++)
      {
        Assert::AreEqual(source[i].size(), parsed[i].size());
        for (size_t j = 0; j < source[i].size(); j++)
        {
          Assert::AreEqual(source[i][j], parsed[i][j]);
        }
      }
    }

    TEST_METHOD(TestMapWithVectorValues)
    {
      // Map whose value type is itself a non-trivial container: confirms the associative
      // serializer composes correctly with sequence serializers.
      map<string, vector<int>> source{
        {"primes", {2, 3, 5, 7}},
        {"empty", {}},
        {"single", {42}},
      };
      auto parsed = roundtrip(source);

      Assert::AreEqual(source.size(), parsed.size());
      for (auto& [key, values] : source)
      {
        auto it = parsed.find(key);
        Assert::IsTrue(it != parsed.end(), L"key missing");
        Assert::AreEqual(values.size(), it->second.size());
        for (size_t i = 0; i < values.size(); i++)
        {
          Assert::AreEqual(values[i], it->second[i]);
        }
      }
    }

    TEST_METHOD(TestCustomConverterRoundTrip)
    {
      // Serialize uppercases, deserialize lowercases — round-tripping any input must yield
      // its lowercase form, proving the descriptor invoked our converter (not the default).
      shouting_message source;
      source.tag = "Hello";
      source.count = 7;

      auto bytes = to_bytes(source);
      auto parsed = from_bytes<shouting_message>(bytes);

      Assert::AreEqual<string>("hello", parsed.tag);
      Assert::AreEqual(7, parsed.count);
    }

    TEST_METHOD(TestContainerInsideDescribedObject)
    {
      // End-to-end: a described object whose properties include each container type.
      // Routed through the descriptor (with version prefix), exercises the binary_property
      // path that picks up the binary_serializer specializations.
      package_with_containers source;
      source.tags = { "alpha", "beta" };
      source.sequence = { 7, 8, 9 };
      source.labels = { "x", "y" };
      source.counts = { {"a", 1}, {"b", 2} };

      auto bytes = to_bytes(source);
      auto parsed = from_bytes<package_with_containers>(bytes);

      Assert::AreEqual(source.tags.size(), parsed.tags.size());
      Assert::AreEqual<string>("alpha", parsed.tags[0]);
      Assert::AreEqual<string>("beta", parsed.tags[1]);

      Assert::AreEqual(source.sequence.size(), parsed.sequence.size());
      auto a = source.sequence.begin();
      auto b = parsed.sequence.begin();
      for (; a != source.sequence.end(); ++a, ++b)
      {
        Assert::AreEqual(*a, *b);
      }

      Assert::AreEqual(source.labels.size(), parsed.labels.size());
      Assert::IsTrue(parsed.labels.contains("x"));
      Assert::IsTrue(parsed.labels.contains("y"));

      Assert::AreEqual(source.counts.size(), parsed.counts.size());
      Assert::AreEqual(1, parsed.counts.at("a"));
      Assert::AreEqual(2, parsed.counts.at("b"));
    }
  };
}
