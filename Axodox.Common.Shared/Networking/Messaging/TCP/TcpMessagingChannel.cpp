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
  tcp_messaging_channel::tcp_messaging_channel(tcp_client&& client) :
    _client(move(client))
  { }

  tcp_messaging_channel::~tcp_messaging_channel()
  {
    _client.close();
    _receiver.reset();
    _sender.reset();
  }

  message_task tcp_messaging_channel::send_message(vector<uint8_t>&& message)
  {
    auto messagePromise = make_shared<message_promise>(move(message));
    message_task task{ messagePromise };
    _messagesToSend.add(move(messagePromise));
    return task;
  }

  void tcp_messaging_channel::on_opening()
  {
    auto& socket = _client.client();
    auto name = format("local: {} remote: {}",
      socket.local_address().to_string(),
      socket.remote_address().to_string()
    );

    _sender = make_unique<background_thread>([this] { send_messages(); }, "* TCP sender thread - " + name);
    _receiver = make_unique<background_thread>([this] { receive_messages(); }, "* TCP receiver thread - " + name);
  }

  void tcp_messaging_channel::send_messages()
  {
    try
    {
      auto stream = _client.stream();

      shared_ptr<message_promise> task{};
      while (is_connected())
      {
        if (_messagesToSend.try_get(task, 200ms))
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
    catch (const exception& exception)
    {
      _logger.log(log_severity::error, "Failed to send messages. Reason: {}", exception.what());
    }
    catch (...)
    {
      _logger.log(log_severity::error, "Failed to send messages.");
    }

    _messagesToSend.complete();
    _messagesToSend.clear();

    on_disconnected();
  }

  void tcp_messaging_channel::receive_messages()
  {
    try
    {
      auto stream = _client.stream();

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
    catch (const exception& exception)
    {
      _logger.log(log_severity::error, "Failed to receive messages. Reason: {}", exception.what());
    }
    catch (...)
    {
      _logger.log(log_severity::error, "Failed to receive messages.");
    }

    on_disconnected();
  }
}
