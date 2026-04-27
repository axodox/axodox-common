#pragma once
#include "Networking/Sockets/Addressing/SocketAddressVariant.h"

namespace Axodox::Networking
{
  enum class discovery_message_kind
  {
    discovery_request,
    discovery_response
  };

  struct discovery_message : Storage::serializable
  {
    static Infrastructure::type_registry<discovery_message> derived_types;

    virtual discovery_message_kind type() const = 0;

    virtual void serialize(Storage::stream& stream, Storage::version_t version) const;
    virtual void deserialize(Storage::stream& stream, Storage::version_t version);
  };

  struct discovery_request : discovery_message
  {
    virtual discovery_message_kind type() const;

    std::string id;

    virtual void serialize(Storage::stream& stream, Storage::version_t version) const;
    virtual void deserialize(Storage::stream& stream, Storage::version_t version);
  };

  struct discovery_response : discovery_message
  {
    virtual discovery_message_kind type() const;

    std::string id;
    socket_address_variant address;

    virtual void serialize(Storage::stream& stream, Storage::version_t version) const;
    virtual void deserialize(Storage::stream& stream, Storage::version_t version);
  };
}