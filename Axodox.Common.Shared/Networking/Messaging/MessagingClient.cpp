#include "common_includes.h"
#include "MessagingClient.h"
#include "Infrastructure/Exceptions.h"

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
  { }

  messaging_client::~messaging_client()
  {
    close();
    _connectionThread.reset();
  }

  void messaging_client::on_opening()
  {
    //After a close, the previous connection thread must end before the flag is cleared, or it would carry on connecting
    _connectionThread.reset();
    _isShuttingDown = false;

    _connectionThread = make_unique<background_thread>([&] { connect(); }, "* messaging connection thread");
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
    lock_guard lock{ _mutex };
    return _channel != nullptr && _channel->is_connected();
  }

  void messaging_client::close()
  {
    //The flag is set before the event, so the connection thread sees it once it wakes up
    _isShuttingDown = true;
    _wakeupEvent.set();
    mark_closed();
  }

  void messaging_client::connect()
  {
    while (!_isShuttingDown)
    {
      try
      {
        auto channel = get_channel();

        _wakeupEvent.reset();

        {
          lock_guard lock{ _mutex };
          _channel = move(channel);
        }

        _events.raise(connected, this, _channel.get());

        _channel->message_received(no_revoke, [&](messaging_channel* channel, span<const uint8_t> message) {
          _events.raise(message_received, channel, message);
          });

        _channel->disconnected(no_revoke, [&](messaging_channel* channel) {
          _events.raise(disconnected, this, channel);
          _wakeupEvent.set();
          });

        //A connected handler may have closed the client, then the channel is not opened at all
        if (!_isShuttingDown)
        {
          _channel->open();
          _wakeupEvent.wait();
        }
      }
      catch (...)
      {
        _logger.log(log_severity::error, "Failed to connect. Reason: {}", get_exception_message());
      }

      //Destroyed outside the lock: the channel waits for its threads, and their event handlers may call into this client
      unique_ptr<messaging_channel> channel;
      {
        lock_guard lock{ _mutex };
        channel = move(_channel);
      }
      channel.reset();

      //The disconnect set the wakeup event, reset it so the retry really waits; close sets the flag before the event
      _wakeupEvent.reset();
      if (!_isShuttingDown) _wakeupEvent.wait(1s);
    }
  }
}
