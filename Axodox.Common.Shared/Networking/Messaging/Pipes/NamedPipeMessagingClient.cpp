#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "NamedPipeMessagingClient.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include "NamedPipeMessagingChannel.h"
#include "NamedPipeSecurity.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Networking
{
  const DWORD _busyPipeTimeout = 1000;

  named_pipe_messaging_client::named_pipe_messaging_client(std::string_view name) :
    _name(name)
  { }

  const std::string& named_pipe_messaging_client::name() const
  {
    return _name;
  }

  unique_ptr<messaging_channel> named_pipe_messaging_client::get_channel()
  {
    //Open the pipe; the server identifies this client but cannot impersonate it
    auto path = named_pipe_path(_name);
    winrt::file_handle pipe{ CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr) };
    if (!pipe)
    {
      auto error = GetLastError();
      if (error == ERROR_PIPE_BUSY) WaitNamedPipeW(path.c_str(), _busyPipeTimeout);
      throw runtime_error(format("Could not open pipe {}: error {}.", _name, error));
    }

    //Refuse pipes created by another user under the same name
    if (!named_pipe_security::is_owned_by_current_user(pipe.get()))
    {
      throw runtime_error(format("Pipe {} was not created by the current user.", _name));
    }

    DWORD mode = PIPE_READMODE_MESSAGE;
    winrt::check_bool(SetNamedPipeHandleState(pipe.get(), &mode, nullptr, nullptr));

    _logger.log(log_severity::information, "Connected to pipe {}.", _name);
    return make_unique<named_pipe_messaging_channel>(move(pipe), named_pipe_end::client);
  }
}

#endif
#endif
