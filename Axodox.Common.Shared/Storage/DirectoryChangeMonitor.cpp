#include "common_includes.h"
#include "DirectoryChangeMonitor.h"

#ifdef PLATFORM_WINDOWS
using namespace Axodox::Infrastructure;
using namespace std;
using namespace std::filesystem;
using namespace winrt;

namespace Axodox::Storage
{
  directory_change_monitor::directory_change_monitor(std::span<const std::filesystem::path> directories) :
    directory_changed(_events),
    _directories(directories.begin(), directories.end())
  {
    _exiting.attach(CreateEvent(nullptr, TRUE, FALSE, nullptr));
    check_bool(bool{ _exiting });

    _waitHandles.reserve(_directories.size() + 1);
    _waitHandles.push_back(_exiting.get());

    _notifications.reserve(_directories.size());
    for (const auto& directory : _directories)
    {
      auto handle = FindFirstChangeNotification(
        directory.c_str(),
        TRUE,
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME);

      if (handle == INVALID_HANDLE_VALUE) throw_last_error();

      auto& wrapped = _notifications.emplace_back();
      wrapped.attach(handle);
      _waitHandles.push_back(handle);
    }

    _worker.attach(CreateThread(nullptr, 0, &directory_change_monitor::monitor_changes, this, 0, nullptr));
    check_bool(bool{ _worker });
  }

  directory_change_monitor::~directory_change_monitor()
  {
    if (_exiting) SetEvent(_exiting.get());
    if (_worker) WaitForSingleObject(_worker.get(), INFINITE);
  }

  unsigned long __stdcall directory_change_monitor::monitor_changes(void* context) noexcept
  {
    auto self = static_cast<directory_change_monitor*>(context);

    while (true)
    {
      auto result = WaitForMultipleObjects(static_cast<DWORD>(self->_waitHandles.size()), self->_waitHandles.data(), FALSE, INFINITE);

      if (result == WAIT_OBJECT_0) break;

      if (result >= WAIT_OBJECT_0 + 1 && result < WAIT_OBJECT_0 + self->_waitHandles.size())
      {
        auto index = result - WAIT_OBJECT_0 - 1;
        self->_events.raise(self->directory_changed, self, self->_directories[index]);

        if (!FindNextChangeNotification(self->_waitHandles[index + 1])) break;
      }
      else
      {
        break;
      }
    }

    return 0;
  }
}
#endif
