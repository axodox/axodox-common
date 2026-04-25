#include "common_includes.h"
#include "MessagingClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;
using namespace chrono_literals;

namespace Axodox::Networking
{
  messaging_client::messaging_client() :
    connected(_events),
    message_received(_events),
    disconnected(_events)
  {
  }

  messaging_client::~messaging_client()
  {
    _is_disposed = true;
    _wakeup.set();
    _connection_thread.reset();
  }

  void messaging_client::on_opening()
  {
    _connection_thread = make_unique<background_thread>([&] { connect(); }, "* messaging connection thread");
  }

  message_task messaging_client::send_message(vector<uint8_t>&& message)
  {
    lock_guard lock{ _mutex };
    if (_channel)
    {
      return _channel->send_message(move(message));
    }
    else
    {
      return message_task{};
    }
  }

  bool messaging_client::is_connected() const
  {
    return _channel != nullptr;
  }

  void messaging_client::connect()
  {
    while (!_is_disposed)
    {
      try
      {
        auto channel = get_client();

        _wakeup.reset();

        {
          lock_guard lock{ _mutex };
          _channel = move(channel);
        }

        _events.raise(connected, this, _channel.get());

        _channel->message_received(no_revoke, [&](messaging_channel*, span<const uint8_t> message) {
          _events.raise(message_received, this, message);
          });

        _channel->disconnected(no_revoke, [&](messaging_channel* channel) {
          _events.raise(disconnected, this, channel);
          _wakeup.set();
          });

        _channel->open();

        _wakeup.wait();
      }
      catch (const exception& exception)
      {
        _logger.log(log_severity::error, "Failed to connect. Reason: {}", exception.what());
      }
      catch (...)
      {
        _logger.log(log_severity::error, "Failed to connect.");
      }

      {
        lock_guard lock{ _mutex };
        _channel.reset();
      }

      if (!_is_disposed) _wakeup.wait(1s);
    }
  }
}
