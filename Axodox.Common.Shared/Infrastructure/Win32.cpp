#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "Win32.h"

using namespace winrt;

namespace Axodox::Infrastructure
{
  std::wstring to_wstring(std::string_view text)
  {
    const auto expectedSize = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int32_t>(text.size()), nullptr, 0);

    if (expectedSize == 0)
    {
      return {};
    }

    std::wstring result(expectedSize, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int32_t>(text.size()), result.data(), expectedSize);
    return result;
  }

  bool has_package_identity()
  {
    uint32_t length = 0u;
    return GetCurrentPackageFullName(&length, nullptr) != APPMODEL_ERROR_NO_PACKAGE;
  }

  winrt::guid make_guid()
  {
    guid result;
    check_hresult(CoCreateGuid(reinterpret_cast<GUID*>(&result)));
    return result;
  }
}
#endif