#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "Win32UserSecurity.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <aclapi.h>
#include <sddl.h>

using namespace std;
using namespace winrt;

namespace
{
  vector<uint8_t> get_current_user_token()
  {
    handle token;
    check_bool(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, token.put()));

    DWORD size = 0;
    GetTokenInformation(token.get(), TokenUser, nullptr, 0, &size);

    vector<uint8_t> buffer(size);
    check_bool(GetTokenInformation(token.get(), TokenUser, buffer.data(), size, &size));
    return buffer;
  }

  PSID get_user_sid(vector<uint8_t>& tokenUser)
  {
    return reinterpret_cast<TOKEN_USER*>(tokenUser.data())->User.Sid;
  }
}

namespace Axodox::Security
{
  win32_user_security::win32_user_security()
  {
    //Grant generic all to the current user and nobody else, with inheritance from the parent blocked
    auto tokenUser = get_current_user_token();

    wchar_t* sid = nullptr;
    check_bool(ConvertSidToStringSidW(get_user_sid(tokenUser), &sid));
    auto sddl = std::format(L"D:P(A;;GA;;;{})", sid);
    LocalFree(sid);

    check_bool(ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl.c_str(), SDDL_REVISION_1, &_descriptor, nullptr));

    _attributes.nLength = sizeof(_attributes);
    _attributes.lpSecurityDescriptor = _descriptor;
    _attributes.bInheritHandle = FALSE;
  }

  win32_user_security::~win32_user_security()
  {
    LocalFree(_descriptor);
  }

  SECURITY_ATTRIBUTES* win32_user_security::attributes()
  {
    return &_attributes;
  }

  bool is_owned_by_current_user(HANDLE handle)
  {
    PSID owner = nullptr;
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    if (GetSecurityInfo(handle, SE_KERNEL_OBJECT, OWNER_SECURITY_INFORMATION, &owner, nullptr, nullptr, nullptr, &descriptor) != ERROR_SUCCESS) return false;

    auto tokenUser = get_current_user_token();
    auto result = EqualSid(owner, get_user_sid(tokenUser)) != FALSE;

    LocalFree(descriptor);
    return result;
  }
}

#endif
#endif
