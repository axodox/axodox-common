#pragma once
#include "Infrastructure/TypeRegistry.h"
#include "Infrastructure/ValuePtr.h"
#include "ArrayStream.h"
#include "MemoryStream.h"

namespace Axodox::Storage
{
  typedef uint16_t version_t;

  template<typename T>
  constexpr version_t version_of()
  {
    if constexpr (requires{ { T::version } -> std::convertible_to<version_t>; })
    {
      return T::version;
    }
    else
    {
      return 0;
    }
  }

  class AXODOX_COMMON_API serializable
  {
  public:
    virtual void serialize(stream& stream, version_t version) const = 0;
    virtual void deserialize(stream& stream, version_t version) = 0;

    virtual ~serializable() = default;
  };

  template <typename T>
  memory_stream to_stream(const T& value)
  {
    memory_stream stream;
    serialize(stream, value);
    return stream;
  }

  template <typename T>
  T from_stream(stream& stream)
  {
    T value;
    deserialize(stream, value);
    return value;
  }

  template <typename T>
  std::vector<uint8_t> to_bytes(const T& value)
  {
    return to_stream(value);
  }

  template <typename T>
  T from_bytes(std::span<const uint8_t> bytes)
  {
    array_stream stream{ bytes };
    return from_stream<T>(stream);
  }

  template<Infrastructure::is_pointing T>
  void serialize_pointer(stream& stream, const T& value, version_t version)
  {
    if (value == nullptr)
    {
      throw std::logic_error("Attempting to serialize nullptr.");
    }

    if constexpr (Infrastructure::has_derived_types<Infrastructure::pointed_t<T>>)
    {
      stream.write(value->Type());
    }

    value->serialize(stream, version);
  }

  template<Infrastructure::is_pointing T>
  void deserialize_pointer(stream& stream, T& value, version_t version)
  {
    if constexpr (Infrastructure::has_derived_types<typename T::element_type>)
    {
      auto id = stream.read<decltype(value->Type())>();
      if constexpr (Infrastructure::instantiation_of<T, Infrastructure::value_ptr>)
      {
        value = T::element_type::actual_types.create_value(id);
      }
      else if constexpr (Infrastructure::instantiation_of<T, std::unique_ptr>)
      {
        value = T::element_type::actual_types.create_unique(id);
      }
      else if constexpr (Infrastructure::instantiation_of<T, std::shared_ptr>)
      {
        value = T::element_type::actual_types.create_shared(id);
      }
      else
      {
        static_assert(false, "Unsupported pointer type");
      }
    }
    else
    {
      static_assert(!std::is_abstract<typename T::element_type>::value, "Cannot deserialize abstract class!");
      if constexpr (Infrastructure::instantiation_of<T, Infrastructure::value_ptr>)
      {
        value = Infrastructure::make_value<typename T::element_type>();
      }
      else if constexpr (Infrastructure::instantiation_of<T, std::unique_ptr>)
      {
        value = std::make_unique<typename T::element_type>();
      }
      else if constexpr (Infrastructure::instantiation_of<T, std::shared_ptr>)
      {
        value = std::make_shared<typename T::element_type>();
      }
      else
      {
        static_assert(false, "Unsupported pointer type");
      }
    }
    value->deserialize(stream, version);
  }

  template<typename T>
  void serialize(stream& stream, const T& value, version_t version)
  {
    if constexpr (Infrastructure::is_pointing<T>)
    {
      serialize_pointer(stream, value, version);
    }
    else if constexpr (std::derived_from<T, serializable>)
    {
      auto& serializableValue = static_cast<const serializable&>(value);
      serializableValue.serialize(stream, version);
    }
    else
    {
      stream.write(value);
    }
  }

  template<typename T>
  void serialize(stream& stream, const T& value)
  {
    version_t version = 0;
    if constexpr (Infrastructure::is_pointing<T>)
    {
      version = version_of<Infrastructure::pointed_t<T>>();
    }
    else if constexpr (Infrastructure::instantiation_of<T, std::vector>)
    {
      version = version_of<typename T::value_type::element_type>();
    }

    stream.write(version);
    serialize(stream, value, version);
  }

  template<typename T>
  void deserialize(stream& stream, T& value, version_t version)
  {
    if constexpr (Infrastructure::is_pointing<T>)
    {
      deserialize_pointer(stream, value, version);
    }
    else if constexpr (std::derived_from<T, serializable>)
    {
      auto& serializableValue = static_cast<serializable&>(value);
      serializableValue.deserialize(stream, version);
    }
    else
    {
      stream.read(value);
    }
  }

  template<typename T>
  void deserialize(stream& stream, T& value)
  {
    auto version = stream.read<version_t>();
    deserialize(stream, value, version);
  }

  template<typename TItem>
  void serialize(stream& stream, const std::vector<TItem>& values, version_t version)
  {
    stream.write((uint32_t)values.size());

    if constexpr (Infrastructure::trivially_copyable<TItem>)
    {
      stream.write(std::span{ reinterpret_cast<const TItem*>(values.data()), values.size() * sizeof(TItem) });
    }
    else
    {
      for (auto& value : values)
      {
        serialize(stream, value, version);
      }
    }
  }

  template<typename TItem>
  void deserialize(stream& stream, std::vector<TItem>& values, version_t version)
  {
    auto count = stream.read<uint32_t>();
    values.resize(count);

    if constexpr (Infrastructure::trivially_copyable<TItem>)
    {
      stream.read(std::span{ reinterpret_cast<TItem*>(values.data()), values.size() * sizeof(TItem) });
    }
    else
    {
      for (auto& value : values)
      {
        deserialize(stream, value, version);
      }
    }
  }

  template<typename TKey, typename TValue>
  void serialize(stream& stream, const std::unordered_map<TKey, TValue>& values, version_t version)
  {
    stream.write((uint32_t)values.size());
    for (auto& [key, value] : values)
    {
      serialize(stream, key, version);
      serialize(stream, value, version);
    }
  }

  template<typename TKey, typename TValue>
  void deserialize(stream& stream, std::unordered_map<TKey, TValue>& values, version_t version)
  {
    values.clear();
    auto count = stream.read<uint32_t>();
    for (auto i = 0u; i < count; i++)
    {
      TKey key;
      TValue value;
      deserialize(stream, key, version);
      deserialize(stream, value, version);
      values.emplace(std::move(key), std::move(value));
    }
  }

  template<typename TValue>
  TValue clone(const TValue& value)
  {
    memory_stream stream;
    serialize(stream, value);
    stream.seek(0);

    TValue result{};
    deserialize(stream, result);
    return result;
  }
}
