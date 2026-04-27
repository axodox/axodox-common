#pragma once
#include "Storage/Stream.h"
#include "Socket.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API socket_stream : public Storage::stream
  {
  public:
    explicit socket_stream(socket& socket);

    using Storage::stream::write;
    using Storage::stream::read;

    virtual void write(std::span<const uint8_t> buffer) override;
    virtual size_t read(std::span<uint8_t> buffer, bool partial = false) override;

    virtual size_t position() const override;
    virtual void seek(size_t position) override;
    virtual size_t length() const override;

  private:
    socket& _socket;
  };
}
