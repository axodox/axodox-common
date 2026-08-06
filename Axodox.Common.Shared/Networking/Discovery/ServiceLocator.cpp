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

    //An announcement on a wildcard host ([::] / 0.0.0.0) means "reachable on any interface",
    //so keep the announced port but take the host from wherever the response came from -
    //that way callers always receive a routable address.
    auto resolvedAddress = response->address;
    if (resolvedAddress.is_any() && addressedMessage.address)
    {
      resolvedAddress = addressedMessage.address;
      resolvedAddress.port(response->address.port());
    }

    service_address eventArgs{
      .id = response->id,
      .address = resolvedAddress
    };
    _events.raise(service_found, this, eventArgs);
  }
}
