#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "NamedPipeMessagingClient.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include "Security/Win32UserSecurity.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Security;
using namespace std;
using namespace winrt;

namespace
{
  //The server identifies this client but cannot impersonate it
  file_handle open_pipe(const std::wstring& path)
  {
    return file_handle{ CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr) };
  }
}

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
    auto path = named_pipe_messaging_channel::pipe_path(_name);
    auto pipe = open_pipe(path);
    auto error = pipe ? ERROR_SUCCESS : GetLastError();

    //Every instance is busy while the server is between accepting a client and creating the next instance
    if (error == ERROR_PIPE_BUSY && WaitNamedPipeW(path.c_str(), _busyPipeTimeout))
    {
      pipe = open_pipe(path);
      error = pipe ? ERROR_SUCCESS : GetLastError();
    }

    if (!pipe) throw runtime_error(format("Could not open pipe {}: error {}.", _name, error));

    //Refuse pipes created by another user under the same name
    if (!is_owned_by_current_user(pipe.get()))
    {
      throw runtime_error(format("Pipe {} was not created by the current user.", _name));
    }

    DWORD mode = PIPE_READMODE_MESSAGE;
    check_bool(SetNamedPipeHandleState(pipe.get(), &mode, nullptr, nullptr));

    _logger.log(log_severity::information, "Connected to pipe {}.", _name);
    return make_unique<named_pipe_messaging_channel>(move(pipe), named_pipe_end::client);
  }
}

#endif
#endif
