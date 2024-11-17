#pragma once
#include "Infrastructure/Traits.h"

namespace Axodox::Storage
{
  class AXODOX_COMMON_API stream
  {
  public:
    virtual void write(std::span<const uint8_t> buffer) = 0;
    virtual size_t read(std::span<uint8_t> buffer, bool partial = false) = 0;

    virtual size_t position() const = 0;
    virtual void seek(size_t position) = 0;

    virtual size_t length() const = 0;
    virtual ~stream() = default;

    template<typename T>
    void read(std::span<T> value)
    {
      read({ reinterpret_cast<uint8_t*>(value.data()), value.size_bytes() });
    }

    template<typename T>
    void write(std::span<const T> value)
    {
      write({ reinterpret_cast<const uint8_t*>(value.data()), value.size_bytes() });
    }

    template<typename T>
    requires Infrastructure::trivially_copyable<T>
    void read(std::vector<T>& value)
    {
      uint32_t length;
      read(length);

      value.resize(length);
      read({ reinterpret_cast<uint8_t*>(value.data()), value.size() * sizeof(T) });
    }

    template<typename T>
    requires Infrastructure::trivially_copyable<T>
    void write(const std::vector<T>& value)
    {
      write(uint32_t(value.size()));
      write({ reinterpret_cast<const uint8_t*>(value.data()), value.size() * sizeof(T) });
    }

    template<Infrastructure::trivially_copyable T>
    void read(T& value)
    {
      read({ reinterpret_cast<uint8_t*>(&value), sizeof(T) });
    }

    template<Infrastructure::trivially_copyable T>
    void write(const T& value)
    {
      write({ reinterpret_cast<const uint8_t*>(&value), sizeof(T) });
    }

    void read(std::string& value)
    {
      uint32_t length;
      read(length);

      value.resize(length);
      read({ reinterpret_cast<uint8_t*>(value.data()), value.length() });
    }

    void write(const std::string& value)
    {
      write(uint32_t(value.length()));
      write({ reinterpret_cast<const uint8_t*>(value.data()), value.length() });
    }

    void write(std::string_view value)
    {
      write({ reinterpret_cast<const uint8_t*>(value.data()), value.length() });
    }    

    std::string read_line();

    std::vector<uint8_t> read_to_end();
  };
}