#include "common_includes.h"
#include "ServiceProvider.h"
#include "Discovery.h"
#include "Networking/Sockets/Addressing/SocketAddressV4.h"
#include "Networking/Sockets/Addressing/SocketAddressV6.h"

using namespace Axodox::Storage;
using namespace std;

namespace {
  Axodox::Networking::udp_client make_discovery_client(const Axodox::Networking::socket_address_variant& address)
  {
    auto opts = Axodox::Networking::udp_options{ .is_address_reused = true, .multicast_group = address.address() };
    if (auto v4 = address.as<Axodox::Networking::socket_address_ipv4>())
      return Axodox::Networking::udp_client{ Axodox::Networking::socket_address_ipv4{ Axodox::Networking::ip_address_v4::any, v4->port() }, opts };
    if (auto v6 = address.as<Axodox::Networking::socket_address_ipv6>())
      return Axodox::Networking::udp_client{ Axodox::Networking::socket_address_ipv6{ Axodox::Networking::ip_address_v6::any, v6->port() }, opts };
    return Axodox::Networking::udp_client{ address.port(), opts };
  }
}

namespace Axodox::Networking
{
  service_provider::service_provider(const socket_address_variant& address) :
    _address(address),
    _client(make_discovery_client(address)),
    _messageReceivedSubscription(_client.message_received({ this, &service_provider::on_message_received }))
  { }

  void service_provider::announce(const std::string& id, const socket_address_variant& address)
  {
    unique_lock lock{ _mutex };
    _services[id] = address;

    send_announcement(_address, id, address);
  }

  void service_provider::on_message_received(udp_client* /*sender*/, const udp_addressed_message& addressedMessage)
  {
    //Read request
    auto message = from_bytes<unique_ptr<discovery_message>>(addressedMessage.content);
    if (message->type() != discovery_message_kind::discovery_request) return;

    auto request = static_cast<const discovery_request*>(message.get());

    //Locate service
    shared_lock lock{ _mutex };
    auto it = _services.find(request->id);
    if (it == _services.end()) return;

    //Send response
    send_announcement(_address, it->first, it->second); //We cannot just send response to sender, as then multiple apps using the same port will not receive it
  }

  void service_provider::send_announcement(const socket_address& target, const std::string& id, const socket_address_variant& address)
  {
    //Write response
    discovery_response response;
    response.id = id;
    response.address = address;

    auto content = to_bytes(&response);

    //Send response
    udp_addressed_message message{
      .address = target,
      .content = content
    };

    _client.send_message(message);
  }
}