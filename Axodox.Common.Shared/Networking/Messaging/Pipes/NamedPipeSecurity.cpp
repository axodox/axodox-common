#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "NamedPipeSecurity.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include "Infrastructure/Win32.h"
#include <aclapi.h>
#include <sddl.h>

using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  vector<uint8_t> get_current_user_token()
  {
    winrt::handle token;
    winrt::check_bool(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, token.put()));

    DWORD size = 0;
    GetTokenInformation(token.get(), TokenUser, nullptr, 0, &size);

    vector<uint8_t> buffer(size);
    winrt::check_bool(GetTokenInformation(token.get(), TokenUser, buffer.data(), size, &size));
    return buffer;
  }

  PSID get_user_sid(vector<uint8_t>& tokenUser)
  {
    return reinterpret_cast<TOKEN_USER*>(tokenUser.data())->User.Sid;
  }
}

namespace Axodox::Networking
{
  named_pipe_security::named_pipe_security()
  {
    //Grant generic all to the current user and nobody else, with inheritance from the parent blocked
    auto tokenUser = get_current_user_token();

    wchar_t* sid = nullptr;
    winrt::check_bool(ConvertSidToStringSidW(get_user_sid(tokenUser), &sid));
    auto sddl = format(L"D:P(A;;GA;;;{})", sid);
    LocalFree(sid);

    winrt::check_bool(ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl.c_str(), SDDL_REVISION_1, &_descriptor, nullptr));

    _attributes.nLength = sizeof(_attributes);
    _attributes.lpSecurityDescriptor = _descriptor;
    _attributes.bInheritHandle = FALSE;
  }

  named_pipe_security::~named_pipe_security()
  {
    LocalFree(_descriptor);
  }

  SECURITY_ATTRIBUTES* named_pipe_security::attributes()
  {
    return &_attributes;
  }

  bool named_pipe_security::is_owned_by_current_user(HANDLE pipe)
  {
    PSID owner = nullptr;
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    if (GetSecurityInfo(pipe, SE_KERNEL_OBJECT, OWNER_SECURITY_INFORMATION, &owner, nullptr, nullptr, nullptr, &descriptor) != ERROR_SUCCESS) return false;

    auto tokenUser = get_current_user_token();
    auto result = EqualSid(owner, get_user_sid(tokenUser)) != FALSE;

    LocalFree(descriptor);
    return result;
  }

  std::wstring named_pipe_path(std::string_view name)
  {
    return L"\\\\.\\pipe\\" + to_wstring(name);
  }
}

#endif
#endif
