#include "common_includes.h"
#include "MessagingServer.h"
#include "Collections/Extensions.h"

using namespace Axodox::Collections;
using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Networking
{
  messaging_server::messaging_server() :
    client_connected(_events),
    message_received(_events),
    client_disconnected(_events)
  { }

  messaging_server::~messaging_server()
  {
    unique_lock lock{ _mutex };
    _clients.clear();
  }

  uint32_t messaging_server::client_count() const
  {
    return uint32_t(_clients.size());
  }

  Threading::locked_ptr<std::vector<std::unique_ptr<messaging_channel>>> messaging_server::clients()
  {
    return { _mutex, &_clients };
  }

  void messaging_server::broadcast(vector<uint8_t>&& message, messaging_channel* exception)
  {
    shared_lock lock{ _mutex };

    for (auto& client : _clients)
    {
      if (exception == client.get()) continue;

      auto copy{ message };
      client->send_message(move(copy));
    }
  }

  void messaging_server::on_client_connected(unique_ptr<messaging_channel>&& channel)
  {
    channel->message_received(no_revoke, [&](messaging_channel* channel, span<const uint8_t> message) {
      _events.raise(message_received, channel, message);
      });

    channel->disconnected(no_revoke, [&](messaging_channel* channel) {
      unique_lock lock{ _mutex };
      _events.raise(client_disconnected, this, channel);
      _clientParkingSpace = remove_unordered(_clients, channel);
      });

    auto channelPointer = channel.get();

    {
      unique_lock lock{ _mutex };
      _clients.push_back(move(channel));
    }

    _events.raise(client_connected, this, channelPointer);
    channelPointer->open();
  }
}
