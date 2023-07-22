#pragma once
#include "Stream.h"

namespace Axodox::Storage
{
  class AXODOX_COMMON_API memory_stream : public stream
  {
  public:
    virtual void write(std::span<const uint8_t> buffer) override;
    virtual size_t read(std::span<uint8_t> buffer, bool partial = false) override;

    virtual size_t position() const override;
    virtual void seek(size_t position) override;
    virtual size_t length() const override;

    using stream::write;
    using stream::read;

    operator std::span<uint8_t>();
    operator std::span<const uint8_t>() const;
    operator std::vector<uint8_t> && ();

    uint8_t* data();
    const uint8_t* data() const;

    void reserve(size_t size);

  private:
    std::vector<uint8_t> _buffer;
    size_t _position = 0u;

    uint8_t* current();
  };
}