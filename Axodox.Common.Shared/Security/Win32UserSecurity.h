#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Axodox::Security
{
  //Security attributes for objects which only the current user may open, e.g. for CreateNamedPipe, CreateEvent or
  //CreateFileMapping. Without them, a named pipe would also be readable by everyone.
  class AXODOX_COMMON_API win32_user_security
  {
  public:
    win32_user_security();
    ~win32_user_security();

    win32_user_security(const win32_user_security&) = delete;
    win32_user_security& operator=(const win32_user_security&) = delete;

    SECURITY_ATTRIBUTES* attributes();

  private:
    PSECURITY_DESCRIPTOR _descriptor = nullptr;
    SECURITY_ATTRIBUTES _attributes{};
  };

  //Whether the owner of an object (pipe, file, event...) is the current user, so that one created by another user under
  //the same name is not trusted. Objects created by an elevated process are owned by the Administrators group instead.
  AXODOX_COMMON_API bool is_owned_by_current_user(HANDLE handle);
}

#endif
#endif
