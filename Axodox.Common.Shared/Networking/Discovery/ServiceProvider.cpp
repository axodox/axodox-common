#include "common_includes.h"
#include "ServiceProvider.h"
#include "Discovery.h"

using namespace Axodox::Storage;
using namespace std;

namespace Axodox::Networking
{
  service_provider::service_provider(const socket_address_variant& address) :
    _address(address),
    _client(address.port(), udp_options{ .is_address_reused = true, .multicast_group = address.address() }),
    _messageReceivedSubscription(_client.message_received({ this, &service_provider::on_message_received }))
  { }

  void service_provider::announce(const std::string& id, const socket_address_variant& address)
  {
    unique_lock lock{ _mutex };
    _services[id] = address;

    _logger.log(Infrastructure::log_severity::information, "Announcing service '{}' at {}.", id, address.to_string());
    send_announcement(_address, id, address);
  }

  void service_provider::on_message_received(udp_client* /*sender*/, const udp_addressed_message& addressedMessage)
  {
    //Read request
    auto message = from_bytes<unique_ptr<discovery_message>>(addressedMessage.content);
    if (message->type() != discovery_message_kind::discovery_request) return;

    auto request = static_cast<const discovery_request*>(message.get());

    _logger.log(Infrastructure::log_severity::information, "Received discovery request for '{}' from {}.", request->id, addressedMessage.address.to_string());

    //Locate service
    shared_lock lock{ _mutex };
    auto it = _services.find(request->id);
    if (it == _services.end())
    {
      _logger.log(Infrastructure::log_severity::information, "No service registered for '{}', ignoring request.", request->id);
      return;
    }

    _logger.log(Infrastructure::log_severity::information, "Responding to discovery request for '{}' with announced address {}.", it->first, it->second.to_string());

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