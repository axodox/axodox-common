#include "common_includes.h"
#include "MessagingChannel.h"

using namespace std;

namespace Axodox::Networking
{
  messaging_channel::messaging_channel() :
    message_received(_events),
    disconnected(_events)
  { }

  messaging_channel::~messaging_channel()
  {
    on_disconnected();
  }

  bool messaging_channel::is_connected() const
  {
    return _isConnected;
  }

  void messaging_channel::on_received(span<const uint8_t> message)
  {
    _events.raise(message_received, this, message);
  }

  void messaging_channel::on_disconnected()
  {
    if (!_isConnected) return;

    lock_guard<mutex> lock{ _mutex };
    if (_isConnected)
    {
      _isConnected = false;
      _events.raise(disconnected, this);
    }
  }
}
