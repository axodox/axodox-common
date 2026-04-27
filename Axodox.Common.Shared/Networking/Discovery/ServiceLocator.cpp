#include "common_includes.h"
#include "ServiceLocator.h"
#include "Discovery.h"

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

    service_address eventArgs{
      .id = response->id,
      .address = response->address
    };
    _events.raise(service_found, this, eventArgs);
  }
}