#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "NamedPipeMessagingServer.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include "NamedPipeMessagingChannel.h"
#include "Infrastructure/Exceptions.h"
#include "Threading/Win32OverlappedOperation.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;
using namespace winrt;
using namespace std::chrono_literals;

namespace Axodox::Networking
{
  const DWORD _pipeBufferSize = 64 * 1024;

  named_pipe_messaging_server::named_pipe_messaging_server(std::string_view name) :
    _name(name)
  { }

  named_pipe_messaging_server::~named_pipe_messaging_server()
  {
    _stopEvent.set();
    _listener.reset();
  }

  const std::string& named_pipe_messaging_server::name() const
  {
    return _name;
  }

  void named_pipe_messaging_server::on_opening()
  {
    _waitingPipe = create_pipe(true);
    _listener = make_unique<background_thread>([this] { accept_clients(); }, "* pipe listener - " + _name);
    _logger.log(log_severity::information, "Listening on pipe {}.", _name);
  }

  file_handle named_pipe_messaging_server::create_pipe(bool isFirstInstance)
  {
    auto path = named_pipe_messaging_channel::pipe_path(_name);
    file_handle result{ CreateNamedPipeW(
      path.c_str(),
      PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | (isFirstInstance ? FILE_FLAG_FIRST_PIPE_INSTANCE : 0),
      PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
      PIPE_UNLIMITED_INSTANCES,
      _pipeBufferSize,
      _pipeBufferSize,
      0,
      _security.attributes()) };

    if (!result) throw runtime_error(format("Could not create pipe {}: error {}.", _name, GetLastError()));
    return result;
  }

  void named_pipe_messaging_server::accept_clients()
  {
    win32_overlapped_operation operation;
    while (!_stopEvent)
    {
      try
      {
        //Create the instance for the next client: the previous one went to its channel, or was dropped after a failure
        if (!_waitingPipe) _waitingPipe = create_pipe(false);

        //Wait for a client on the waiting instance
        auto isStarted = ConnectNamedPipe(_waitingPipe.get(), operation.get()) != FALSE;

        uint32_t transferred;
        auto error = operation.wait(_waitingPipe.get(), isStarted, _stopEvent, transferred);
        if (error == ERROR_OPERATION_ABORTED) break;

        //A client which connected before ConnectNamedPipe was called is reported as ERROR_PIPE_CONNECTED
        if (error == ERROR_SUCCESS || error == ERROR_PIPE_CONNECTED)
        {
          on_client_connected(make_unique<named_pipe_messaging_channel>(move(_waitingPipe), named_pipe_end::server));
        }
        else
        {
          _logger.log(log_severity::warning, "Accepting a client failed with error {}.", error);
          _waitingPipe.close();
        }
      }
      catch (...)
      {
        _logger.log(log_severity::error, "Failed to accept clients. Reason: {}", get_exception_message());
        _stopEvent.wait(1s);
      }
    }
  }
}

#endif
#endif
