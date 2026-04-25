#include "common_includes.h"
#include "MessagingChannel.h"

using namespace std;

namespace Axodox::Networking
{
  messaging_channel::messaging_channel() :
    message_received(_events),
    disconnected(_events)
  { }

  bool messaging_channel::is_connected() const
  {
    return _is_connected;
  }

  void messaging_channel::on_received(span<const uint8_t> message)
  {
    _events.raise(message_received, this, message);
  }

  void messaging_channel::on_disconnected()
  {
    if (!_is_connected) return;

    lock_guard<mutex> lock{ _mutex };
    if (_is_connected)
    {
      _is_connected = false;
      _events.raise(disconnected, this);
    }
  }
}
