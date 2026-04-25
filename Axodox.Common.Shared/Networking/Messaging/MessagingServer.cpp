#include "common_includes.h"
#include "MessagingServer.h"
#include "Collections/Extensions.h"
#include "Threading/Parallel.h"

using namespace Axodox::Collections;
using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
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
    _isShuttingDown = true;
    _clients.clear();
  }

  uint32_t messaging_server::client_count() const
  {
    return uint32_t(_clients.size());
  }

  Threading::locked_ptr<const std::vector<std::unique_ptr<messaging_channel>>> messaging_server::clients() const
  {
    return { _mutex, &_clients };
  }

  void messaging_server::broadcast_message(vector<uint8_t>&& message, messaging_channel* exception)
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
    //Set up event handlers
    channel->message_received(no_revoke, { this, &messaging_server::on_message_received });
    channel->disconnected(no_revoke, { this, &messaging_server::on_client_disconnected });

    //Register client
    auto client = channel.get();

    {
      unique_lock lock{ _mutex };
      _clients.push_back(move(channel));
    }

    //Raise connected
    _events.raise(client_connected, this, client);

    //Start client
    client->open();
  }

  void messaging_server::on_message_received(messaging_channel* channel, std::span<const uint8_t> message)
  {
    _events.raise(message_received, channel, message);
  }

  void messaging_server::on_client_disconnected(messaging_channel* channel)
  {
    //Remove the client
    unique_ptr<messaging_channel> client;
    if (!_isShuttingDown) //When shutting down cleanup is done in the destructor - also we do not want to deadlock
    {
      unique_lock lock{ _mutex };
      client = remove_unordered(_clients, channel);
    }

    //Raise disconnected
    _events.raise(client_disconnected, this, channel);

    //Destroy the channel async - we cannot delete it from its own event handler, as that would deadlock
    if (client)
    {
      delete_async(move(client));
    }
  }
}
