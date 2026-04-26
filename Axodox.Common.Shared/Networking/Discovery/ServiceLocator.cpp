#include "common_includes.h"
#include "ServiceLocator.h"
#include "Discovery.h"

using namespace Axodox::Storage;

namespace Axodox::Networking
{
  service_locator::service_locator(const socket_address_variant& address) :
    service_found(_events),
    _address(address),
    _client(address.port(), udp_options{ .multicast_group = address.address() }),
    _messageReceivedSubscription(_client.message_received({ this, &service_locator::on_message_received }))
  { }

  void service_locator::locate_service(const std::string& id)
  {
    //Write request
    discovery_request request;
    request.id = id;

    auto content = to_bytes(request);

    //Send request
    udp_addressed_message message{
      .address = _address,
      .content = content
    };

    _client.send_message(message);
  }

  void service_locator::on_message_received(udp_client* /*sender*/, const udp_addressed_message& message)
  {
    auto request = from_bytes<discovery_response>(message.content);

    service_address eventArgs{
      .id = request.id,
      .address = request.address
    };
    _events.raise(service_found, this, eventArgs);
  }
}