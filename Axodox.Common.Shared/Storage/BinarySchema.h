#pragma once
#include "Serializer.h"

namespace Axodox::Storage
{
  using type_id_t = uint16_t;

#pragma region Serializer infrastructure
  template <typename value_t>
  struct binary_serializer
  {
    void serialize(stream& stream, const value_t& value) const
    {
      stream.write(value);
    }

    void deserialize(stream& stream, value_t& value) const
    {
      stream.read(value);
    }
  };

  //A binary_converter for value_t is anything callable like binary_serializer<value_t>:
  //a const-qualified serialize(stream&, const value_t&) and deserialize(stream&, value_t&).
  //Used to constrain custom converters against the field type they target so a converter
  //for one type cannot be silently bound to a property of another.
  template <typename converter_t, typename value_t>
  concept binary_converter = requires(const converter_t & c, stream & s, const value_t & cv, value_t & v)
  {
    { c.serialize(s, cv) } -> std::same_as<void>;
    { c.deserialize(s, v) } -> std::same_as<void>;
  };
#pragma endregion

#pragma region Collection serialization
  namespace detail
  {
    //Length-prefixed serializer for sequence and set-like containers. Used via inheritance
    //by the per-template binary_serializer partial specializations below.
    template <typename container_t>
    struct binary_sequence_serializer
    {
      using item_t = typename container_t::value_type;

      void serialize(stream& stream, const container_t& value) const
      {
        stream.write(uint32_t(value.size()));
        binary_serializer<item_t> item_serializer;
        for (auto& item : value)
        {
          item_serializer.serialize(stream, item);
        }
      }

      void deserialize(stream& stream, container_t& value) const
      {
        value.clear();
        auto count = stream.read<uint32_t>();
        binary_serializer<item_t> item_serializer;
        for (auto i = 0u; i < count; i++)
        {
          item_t item;
          item_serializer.deserialize(stream, item);
          if constexpr (requires(container_t c, item_t i) { c.push_back(std::move(i)); })
          {
            value.push_back(std::move(item));
          }
          else
          {
            value.insert(std::move(item));
          }
        }
      }
    };

    //Length-prefixed serializer for keyed associative containers (map / unordered_map).
    template <typename container_t>
    struct binary_associative_serializer
    {
      using key_t = typename container_t::key_type;
      using value_t = typename container_t::mapped_type;

      void serialize(stream& stream, const container_t& value) const
      {
        stream.write(uint32_t(value.size()));
        binary_serializer<key_t> key_serializer;
        binary_serializer<value_t> value_serializer;
        for (auto& [k, v] : value)
        {
          key_serializer.serialize(stream, k);
          value_serializer.serialize(stream, v);
        }
      }

      void deserialize(stream& stream, container_t& value) const
      {
        value.clear();
        auto count = stream.read<uint32_t>();
        binary_serializer<key_t> key_serializer;
        binary_serializer<value_t> value_serializer;
        for (auto i = 0u; i < count; i++)
        {
          key_t k;
          value_t v;
          key_serializer.deserialize(stream, k);
          value_serializer.deserialize(stream, v);
          value.emplace(std::move(k), std::move(v));
        }
      }
    };
  }

  //Sequence and set-like containers. std::vector keeps the trivially-copyable bulk fast path
  //via the primary template (stream::read/write have overloads for it); when the item type is
  //not trivially copyable, the partial specialization here takes over.
  template <typename item_t, typename allocator_t>
    requires (!Infrastructure::trivially_copyable<item_t>)
  struct binary_serializer<std::vector<item_t, allocator_t>>
    : detail::binary_sequence_serializer<std::vector<item_t, allocator_t>>
  { };

  template <typename item_t, typename allocator_t>
  struct binary_serializer<std::list<item_t, allocator_t>>
    : detail::binary_sequence_serializer<std::list<item_t, allocator_t>>
  { };

  template <typename item_t, typename compare_t, typename allocator_t>
  struct binary_serializer<std::set<item_t, compare_t, allocator_t>>
    : detail::binary_sequence_serializer<std::set<item_t, compare_t, allocator_t>>
  { };

  template <typename item_t, typename hash_t, typename equal_t, typename allocator_t>
  struct binary_serializer<std::unordered_set<item_t, hash_t, equal_t, allocator_t>>
    : detail::binary_sequence_serializer<std::unordered_set<item_t, hash_t, equal_t, allocator_t>>
  { };

  //Keyed associative containers.
  template <typename key_t, typename value_t, typename compare_t, typename allocator_t>
  struct binary_serializer<std::map<key_t, value_t, compare_t, allocator_t>>
    : detail::binary_associative_serializer<std::map<key_t, value_t, compare_t, allocator_t>>
  { };

  template <typename key_t, typename value_t, typename hash_t, typename equal_t, typename allocator_t>
  struct binary_serializer<std::unordered_map<key_t, value_t, hash_t, equal_t, allocator_t>>
    : detail::binary_associative_serializer<std::unordered_map<key_t, value_t, hash_t, equal_t, allocator_t>>
  { };
#pragma endregion

#pragma region Object serialization
  struct binary_property_options
  {
    version_t min_version = 0;
    version_t max_version = version_t(~0u);
  };

  class binary_property_descriptor_base
  {
  public:
    void serialize(stream& stream, const void* object) const
    {
      _serialize(stream, object);
    }

    void deserialize(stream& stream, void* object) const
    {
      _deserialize(stream, object);
    }

    const binary_property_options& options() const
    {
      return _options;
    }

  protected:
    template<typename object_t, typename value_t, typename converter_t = binary_serializer<value_t>>
      requires binary_converter<converter_t, value_t>
    binary_property_descriptor_base(value_t object_t::* field, const binary_property_options& options = {}, converter_t converter = {}) :
      _serialize([=](stream& stream, const void* object) { converter.serialize(stream, static_cast<const object_t*>(object)->*field); }),
      _deserialize([=](stream& stream, void* object) { converter.deserialize(stream, static_cast<object_t*>(object)->*field); }),
      _options(options)
    {
    }

  private:
    std::function<void(stream&, const void*)> _serialize;
    std::function<void(stream&, void*)> _deserialize;
    binary_property_options _options;
  };

  template<typename object_t>
  class binary_property_descriptor : public binary_property_descriptor_base
  {
  public:
    template<typename value_t, typename converter_t = binary_serializer<value_t>>
      requires binary_converter<converter_t, value_t>
    binary_property_descriptor(value_t object_t::* field, const binary_property_options& options = {}, converter_t converter = {}) :
      binary_property_descriptor_base(field, options, converter)
    {
    }
  };

  struct binary_object_options
  {
    type_id_t type_id = type_id_t(~0u);
    version_t version = version_t(~0u);
  };

  template<typename object_t>
  concept described_binary_object = requires { { object_t::binary_description.properties() } -> std::convertible_to<const std::vector<binary_property_descriptor_base>&>; };

  template<typename object_t>
  class binary_object_descriptor
  {
    template<typename other_t> friend class binary_object_descriptor;

  public:
    std::type_index index() const { return _index; }
    type_id_t id() const { return _id; }
    version_t version() const { return _version; }
    const std::vector<binary_property_descriptor_base>& properties() const { return _properties; }
    const std::unordered_map<std::type_index, binary_object_descriptor*>& base_descriptors() const { return _base_descriptors; }
    const std::unordered_map<std::type_index, binary_object_descriptor*>& derived_descriptors() const { return _derived_descriptors; }
    std::unique_ptr<object_t> instantiate() const { return _instantiate ? _instantiate() : nullptr; }

    template<typename base_t>
    static binary_object_descriptor create(const binary_object_options& options = {}, std::initializer_list<binary_property_descriptor<object_t>> properties = {})
    {
      binary_object_descriptor result;
      if (options.type_id != type_id_t(~0u)) result._id = options.type_id;
      if (options.version != version_t(~0u)) result._version = options.version;

      if constexpr (std::is_default_constructible_v<object_t>)
      {
        result._instantiate = [] { return std::make_unique<object_t>(); };
      }

      if constexpr (described_binary_object<base_t>)
      {
        //We rely on the base subobject sitting at offset 0 in object_t (single inheritance from base_t,
        //and base_t itself reachable through a chain of zero-offset bases). Multiple inheritance, or any
        //setup that places base_t at a non-zero offset, would break the void*-erased property lambdas.
        constexpr auto sample = std::uintptr_t{ alignof(object_t) };
        if (reinterpret_cast<std::uintptr_t>(static_cast<base_t*>(reinterpret_cast<object_t*>(sample))) != sample)
        {
          throw std::logic_error("Binary object descriptor requires base at offset 0; multiple inheritance is not supported.");
        }

        add_derived(reinterpret_cast<binary_object_descriptor*>(&base_t::binary_description), &object_t::binary_description);

        result._properties.reserve(properties.size() + base_t::binary_description._properties.size());
        result._properties.insert(result._properties.end(), base_t::binary_description._properties.begin(), base_t::binary_description._properties.end());
      }

      result._properties.insert(result._properties.end(), properties.begin(), properties.end());
      return result;
    }

  private:
    std::type_index _index = std::type_index(typeid(object_t));
    type_id_t _id = type_id_t(~0u);
    version_t _version = 0;
    std::vector<binary_property_descriptor_base> _properties;
    std::unordered_map<std::type_index, binary_object_descriptor*> _base_descriptors;
    std::unordered_map<std::type_index, binary_object_descriptor*> _derived_descriptors;
    std::unique_ptr<object_t>(*_instantiate)() = nullptr;

    static void add_derived(binary_object_descriptor* base, binary_object_descriptor* derived)
    {
      for (auto& [_, existing] : base->_derived_descriptors)
      {
        if (existing != derived && existing->_id == derived->_id)
        {
          throw std::logic_error("Duplicate binary object descriptor id.");
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
    void serialize(stream& stream, const object_t& object) const
    {
      stream.write(_version);
      for (auto& property : _properties)
      {
        if (property.options().min_version <= _version && _version <= property.options().max_version)
        {
          property.serialize(stream, &object);
        }
      }
    }

    void deserialize(stream& stream, object_t& object) const
    {
      auto version = stream.read<version_t>();
      for (auto& property : _properties)
      {
        if (property.options().min_version <= version && version <= property.options().max_version)
        {
          property.deserialize(stream, &object);
        }
      }
    }

    template<typename value_t>
      requires Infrastructure::is_pointing<value_t>&& std::convertible_to<Infrastructure::pointed_t<value_t>*, object_t*>
    void serialize(stream& stream, const value_t& object) const
    {
      if (object)
      {
        auto description = this;
        auto index = std::type_index(typeid(*object));
        if (description->_index != index)
        {
          description = description->_derived_descriptors.at(index);
        }

        stream.write(description->_id);
        description->serialize(stream, *object);
      }
      else
      {
        stream.write(type_id_t(~0u));
      }
    }

    template<typename result_t>
      requires requires(std::unique_ptr<object_t> o, result_t r) { r = std::move(o); }
    void deserialize(stream& stream, result_t& object) const
    {
      auto id = stream.read<type_id_t>();
      if (id != type_id_t(~0u))
      {
        auto description = this;
        if (id != _id)
        {
          auto it = std::ranges::find_if(_derived_descriptors, [&id](const auto& value) { return value.second->_id == id; });
          if (it == _derived_descriptors.end()) throw std::runtime_error("Unknown binary object id.");
          description = it->second;
        }

        object = description->instantiate();
        if (!object) throw std::runtime_error("Failed to instantiate binary object. Is the type default constructible?");

        description->deserialize(stream, *object);
      }
      else
      {
        object = nullptr;
      }
    }
  };

  template<typename object_t, typename base_t = void>
  binary_object_descriptor<object_t> describe_binary_object(
    std::initializer_list<binary_property_descriptor<object_t>> properties)
  {
    return binary_object_descriptor<object_t>::template create<base_t>({}, properties);
  }

  template<typename object_t, typename base_t = void>
  binary_object_descriptor<object_t> describe_binary_object(
    const binary_object_options& options,
    std::initializer_list<binary_property_descriptor<object_t>> properties)
  {
    return binary_object_descriptor<object_t>::template create<base_t>(options, properties);
  }
#pragma endregion

#pragma region Serialization extensions
  void serialize(stream& stream, described_binary_object auto const& value)
  {
    value.binary_description.serialize(stream, value);
  }

  void deserialize(stream& stream, described_binary_object auto& value)
  {
    value.binary_description.deserialize(stream, value);
  }

  template <typename value_t>
    requires Infrastructure::is_pointing<value_t>&& described_binary_object<Infrastructure::pointed_t<value_t>>
  void serialize(stream& stream, const value_t& value)
  {
    Infrastructure::pointed_t<value_t>::binary_description.serialize(stream, value);
  }

  template <typename value_t>
    requires Infrastructure::is_pointing<value_t>&& described_binary_object<Infrastructure::pointed_t<value_t>>
  void deserialize(stream& stream, value_t& value)
  {
    Infrastructure::pointed_t<value_t>::binary_description.deserialize(stream, value);
  }
#pragma endregion
}