#include "common_includes.h"
#include "TcpMessagingChannel.h"
#include "Networking/Sockets/SocketStream.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace std;
using namespace std::chrono_literals;

namespace Axodox::Networking
{
  tcp_messaging_channel::tcp_messaging_channel(socket_handle&& socket) :
    _socket(move(socket))
  { }

  message_task tcp_messaging_channel::send_message(vector<uint8_t>&& message)
  {
    auto messagePromise = make_shared<message_promise>(move(message));
    message_task task{ messagePromise };
    _messages_to_send.add(move(messagePromise));
    return task;
  }

  tcp_messaging_channel::~tcp_messaging_channel()
  {
    _socket.reset();
    on_disconnected();
  }

  void tcp_messaging_channel::on_opening()
  {
    _send_thread = make_unique<background_thread>([this] { send_worker(); }, "* TCP sender thread");
    _receive_thread = make_unique<background_thread>([this] { receive_worker(); }, "* TCP receiver thread");
  }

  void tcp_messaging_channel::send_worker()
  {
    try
    {
      socket_stream stream(_socket);

      shared_ptr<message_promise> task{};
      while (is_connected())
      {
        if (_messages_to_send.try_get(task, 200ms))
        {
          stream.write(_magic);
          stream.write(task->message());
          task->finish(true);
          task = nullptr;
        }
        else //heartbeat
        {
          stream.write(_magic);
          stream.write(vector<uint8_t>{});
        }
      }
    }
    catch (...)
    {
      _logger.log(log_severity::error, "TCP send_message failed");
    }

    _messages_to_send.complete();
    _messages_to_send.clear();

    on_disconnected();
  }

  void tcp_messaging_channel::receive_worker()
  {
    try
    {
      socket_stream stream{ _socket };

      while (is_connected())
      {
        auto magic = stream.read<uint64_t>();
        if (magic != _magic)
        {
          throw runtime_error("Magic does not match!");
        }

        auto message = stream.read<vector<uint8_t>>();
        if (message.size() > 0)
        {
          on_received(move(message));
        }
      }
    }
    catch (...)
    {
      _logger.log(log_severity::error, "TCP receive failed");
    }

    on_disconnected();
  }
}
