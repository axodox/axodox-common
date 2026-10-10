#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Axodox::Networking
{
  //Security attributes for named pipes which only the current user may open.
  class AXODOX_COMMON_API named_pipe_security
  {
  public:
    named_pipe_security();
    ~named_pipe_security();

    named_pipe_security(const named_pipe_security&) = delete;
    named_pipe_security& operator=(const named_pipe_security&) = delete;

    SECURITY_ATTRIBUTES* attributes();

    //Whether a pipe was created by the current user, so a client does not talk to a pipe another user created under the same name.
    static bool is_owned_by_current_user(HANDLE pipe);

  private:
    PSECURITY_DESCRIPTOR _descriptor = nullptr;
    SECURITY_ATTRIBUTES _attributes{};
  };

  //The full path of a named pipe: \\.\pipe\<name>
  AXODOX_COMMON_API std::wstring named_pipe_path(std::string_view name);
}

#endif
#endif
