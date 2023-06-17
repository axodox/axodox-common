#include "pch.h"
#ifdef PLATFORM_WINDOWS
#include "UwpStorage.h"

using namespace std;
#ifdef WINRT_Windows_Storage_H
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
#endif

namespace Axodox::Storage
{
#ifdef WINRT_Windows_Storage_H
  std::vector<uint8_t> read_file(const winrt::Windows::Storage::StorageFile& file)
  {
    auto stream = file.OpenReadAsync().get();
    auto buffer = stream.ReadAsync(Buffer{ uint32_t(stream.Size()) }, uint32_t(stream.Size()), InputStreamOptions::None).get();

    vector<uint8_t> result;
    result.resize(buffer.Length());
    memcpy(result.data(), buffer.data(), result.size());

    return result;
  }

  AXODOX_COMMON_API winrt::Windows::Foundation::IAsyncAction read_files_recursively(winrt::Windows::Storage::StorageFolder const& folder, std::vector<winrt::Windows::Storage::StorageFile>& files)
  {
    for (const auto& file : co_await folder.GetFilesAsync())
    {
      files.push_back(file);
    }

    for (const auto& subfolder : co_await folder.GetFoldersAsync())
    {
      co_await read_files_recursively(subfolder, files);
    }
  }
#endif
}
#endif