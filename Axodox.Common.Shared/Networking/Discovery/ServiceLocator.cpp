#include "common_includes.h"
#include "ServiceLocator.h"
#include "Discovery.h"
#include "Networking/Sockets/Addressing/SocketAddressV4.h"
#include "Networking/Sockets/Addressing/SocketAddressV6.h"

using namespace Axodox::Storage;
using namespace std;

namespace {
  Axodox::Networking::udp_client make_discovery_client(const Axodox::Networking::socket_address_variant& address)
  {
    auto udpOptions = Axodox::Networking::udp_options{ .is_address_reused = true, .multicast_group = address.address() };
    if (auto v4 = address.as<Axodox::Networking::socket_address_ipv4>())
      return Axodox::Networking::udp_client{ Axodox::Networking::socket_address_ipv4{ Axodox::Networking::ip_address_v4::any, v4->port() }, udpOptions };
    if (auto v6 = address.as<Axodox::Networking::socket_address_ipv6>())
      return Axodox::Networking::udp_client{ Axodox::Networking::socket_address_ipv6{ Axodox::Networking::ip_address_v6::any, v6->port() }, udpOptions };
    return Axodox::Networking::udp_client{ address.port(), udpOptions };
  }
}

namespace Axodox::Networking
{
  service_locator::service_locator(const socket_address_variant& address) :
    service_found(_events),
    _address(address),
    _client(make_discovery_client(address)),
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
    socket_address_variant resolvedAddress = response->address;
    auto hostVariant = resolvedAddress.address();
    bool isUnspecified =
      holds_alternative<monostate>(hostVariant) ||
      (holds_alternative<ip_address_v4>(hostVariant) && get<ip_address_v4>(hostVariant) == ip_address_v4::any) ||
      (holds_alternative<ip_address_v6>(hostVariant) && get<ip_address_v6>(hostVariant) == ip_address_v6::any);

    if (isUnspecified)
    {
      uint16_t port = resolvedAddress.port();
      if (auto v4 = addressedMessage.address.as<socket_address_ipv4>())
        resolvedAddress = socket_address_ipv4(v4->address(), port);
      else if (auto v6 = addressedMessage.address.as<socket_address_ipv6>())
        resolvedAddress = socket_address_ipv6(v6->address(), port);
    }

    service_address eventArgs{
      .id = response->id,
      .address = resolvedAddress
    };
    _events.raise(service_found, this, eventArgs);
  }
}