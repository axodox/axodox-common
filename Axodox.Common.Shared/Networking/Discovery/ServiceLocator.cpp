#include "common_includes.h"
#include "ServiceLocator.h"
#include "Discovery.h"
#include "Networking/Sockets/Addressing/SocketAddressV4.h"
#include "Networking/Sockets/Addressing/SocketAddressV6.h"

using namespace Axodox::Storage;
using namespace std;

namespace Axodox::Networking
{
  service_locator::service_locator(const socket_address_variant& address) :
    service_found(_events),
    _address(address),
    _client(address.port(), udp_options{ .is_address_reused = true, .multicast_group = address.address() }),
    _messageReceivedSubscription(_client.message_received({ this, &service_locator::on_message_received }))
  { }

  void service_locator::locate_service(const std::string& id)
  {
    //Write request
    discovery_request request;
    request.id = id;

    auto content = to_bytes(&request);

    //Send request
    udp_addressed_message message{
      .address = _address,
      .content = content
    };

    _client.send_message(message);
  }

  void service_locator::on_message_received(udp_client* /*sender*/, const udp_addressed_message& addressedMessage)
  {
    auto message = from_bytes<unique_ptr<discovery_message>>(addressedMessage.content);
    if (message->type() != discovery_message_kind::discovery_response) return;

    auto response = static_cast<const discovery_response*>(message.get());

    // Replace unspecified host ([::] / 0.0.0.0) in the announced address with the
    // actual UDP sender IP, so callers always receive a routable address.
    auto resolvedAddress = resolve_service_address(response->address, addressedMessage.address);

    service_address eventArgs{
      .id = response->id,
      .address = resolvedAddress,
      .sender = addressedMessage.address
    };
    _events.raise(service_found, this, eventArgs);
  }
}
      .address = resolvedAddress
    };
    _events.raise(service_found, this, eventArgs);
  }

  Axodox::Networking::socket_address_variant service_locator::resolve_service_address(
    const Axodox::Networking::socket_address_variant& announced,
    const Axodox::Networking::socket_address_variant& sender)
  {
    auto announcedAddress = announced.address();
    bool isUnspecified =
      holds_alternative<monostate>(announcedAddress) ||
      (holds_alternative<ip_address_v4>(announcedAddress) && get<ip_address_v4>(announcedAddress) == ip_address_v4::any) ||
      (holds_alternative<ip_address_v6>(announcedAddress) && get<ip_address_v6>(announcedAddress) == ip_address_v6::any);

    if (!isUnspecified) return announced;

    uint16_t port = announced.port();
    if (auto senderAddressV4 = sender.as<socket_address_ipv4>()) return socket_address_ipv4(senderAddressV4->address(), port);
    if (auto senderAddressV6 = sender.as<socket_address_ipv6>()) return socket_address_ipv6(senderAddressV6->address(), port);

    return announced;
  }
}
