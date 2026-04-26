#pragma once
#include "Networking/Sockets/Addressing/SocketAddressVariant.h"

namespace Axodox::Networking
{
  struct discovery_request : Storage::serializable
  {
    std::string id;

    virtual void serialize(Storage::stream& stream, Storage::version_t version) const;
    virtual void deserialize(Storage::stream& stream, Storage::version_t version);
  };

  struct discovery_response : Storage::serializable
  {
    std::string id;
    socket_address_variant address;

    virtual void serialize(Storage::stream& stream, Storage::version_t version) const;
    virtual void deserialize(Storage::stream& stream, Storage::version_t version);
  };
}