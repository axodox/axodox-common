#include "common_includes.h"
#include "ServiceProvider.h"
#include "Discovery.h"

using namespace Axodox::Storage;
using namespace std;

namespace Axodox::Networking
{
  service_provider::service_provider(const socket_address_variant& address) :
    _address(address),
    _client(address.port(), udp_options{ .multicast_group = address.address() }),
    _messageReceivedSubscription(_client.message_received({ this, &service_provider::on_message_received }))
  { }

  void service_provider::announce(const std::string& id, const socket_address_variant& address)
  {
    unique_lock lock{ _mutex };
    _services[id] = address;

    send_announcement(_address, id, address);
  }

  void service_provider::on_message_received(udp_client* /*sender*/, const udp_addressed_message& message)
  {
    //Read request
    auto request = from_bytes<discovery_request>(message.content);

    //Locate service
    shared_lock lock{ _mutex };
    auto it = _services.find(request.id);
    if (it == _services.end()) return;

    //Send response
    send_announcement(message.address, it->first, it->second);
  }

  void service_provider::send_announcement(const socket_address& target, const std::string& id, const socket_address_variant& address)
  {
    //Write response
    discovery_response response;
    response.id = id;
    response.address = address;

    auto content = to_bytes(response);

    //Send response
    udp_addressed_message message{
      .address = target,
      .content = content
    };

    _client.send_message(message);
  }
}