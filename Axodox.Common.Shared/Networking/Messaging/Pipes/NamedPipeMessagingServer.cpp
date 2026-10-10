#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "NamedPipeMessagingServer.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include "NamedPipeMessagingChannel.h"
#include "OverlappedOperation.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;

namespace Axodox::Networking
{
  const DWORD _pipeBufferSize = 64 * 1024;

  named_pipe_messaging_server::named_pipe_messaging_server(std::string_view name) :
    _name(name),
    _stopEvent(CreateEventW(nullptr, TRUE, FALSE, nullptr))
  {
    winrt::check_bool(bool(_stopEvent));
  }

  named_pipe_messaging_server::~named_pipe_messaging_server()
  {
    SetEvent(_stopEvent.get());
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

  winrt::file_handle named_pipe_messaging_server::create_pipe(bool isFirstInstance)
  {
    auto path = named_pipe_path(_name);
    winrt::file_handle result{ CreateNamedPipeW(
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
    overlapped_operation operation;
    while (WaitForSingleObject(_stopEvent.get(), 0) != WAIT_OBJECT_0)
    {
      try
      {
        //Wait for a client on the waiting instance
        auto isStarted = ConnectNamedPipe(_waitingPipe.get(), operation.get()) != FALSE;

        uint32_t transferred;
        auto error = operation.wait(_waitingPipe.get(), isStarted, _stopEvent.get(), transferred);
        if (error == ERROR_OPERATION_ABORTED) break;

        //A client which connected before ConnectNamedPipe was called is reported as ERROR_PIPE_CONNECTED
        if (error == ERROR_SUCCESS || error == ERROR_PIPE_CONNECTED)
        {
          on_client_connected(make_unique<named_pipe_messaging_channel>(move(_waitingPipe), named_pipe_end::server));
        }
        else
        {
          _logger.log(log_severity::warning, "Accepting a client failed with error {}.", error);
        }

        //Create the next instance for the next client
        _waitingPipe = create_pipe(false);
      }
      catch (const exception& exception)
      {
        _logger.log(log_severity::error, "Failed to accept clients. Reason: {}", exception.what());
        WaitForSingleObject(_stopEvent.get(), 1000);
      }
    }
  }
}

#endif
#endif
