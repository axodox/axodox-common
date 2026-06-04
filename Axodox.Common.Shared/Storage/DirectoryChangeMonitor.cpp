#include "common_includes.h"
#include "DirectoryChangeMonitor.h"

#ifdef PLATFORM_WINDOWS
#include "Threading/BackgroundThread.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;
using namespace std::filesystem;
using namespace winrt;

namespace
{
  struct find_change_notification_traits
  {
    using type = HANDLE;

    static void close(type value) noexcept
    {
      FindCloseChangeNotification(value);
    }

    static constexpr type invalid() noexcept
    {
      return nullptr;
    }
  };

  using find_change_notification_handle = winrt::handle_type<find_change_notification_traits>;

  std::vector<find_change_notification_handle> register_watches(std::span<const std::filesystem::path> directories)
  {
    std::vector<find_change_notification_handle> notifications;
    notifications.reserve(directories.size());

    for (const auto& directory : directories)
    {
      auto handle = FindFirstChangeNotification(
        directory.c_str(),
        TRUE,
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME);

      if (handle == INVALID_HANDLE_VALUE) throw_last_error();

      notifications.emplace_back(handle);
    }

    return notifications;
  }
}

namespace Axodox::Storage
{
  struct directory_change_monitor::context
  {
    std::vector<std::filesystem::path> directories;
    std::vector<find_change_notification_handle> notifications;
    background_thread thread;

    context(directory_change_monitor* owner, std::span<const std::filesystem::path> directories) :
      directories(directories.begin(), directories.end()),
      notifications(register_watches(directories)),
      thread([this, owner] { owner->monitor_changes(*this); }, "directory change monitor")
    { }
  };

  directory_change_monitor::directory_change_monitor(std::span<const std::filesystem::path> directories) :
    directory_changed(_events),
    _context(make_unique<context>(this, directories))
  { }

  directory_change_monitor::~directory_change_monitor() = default;

  void directory_change_monitor::monitor_changes(context& context) noexcept
  {
    //The thread's wait handle leads the array, so signaling it (on exit) breaks the loop
    vector<HANDLE> waitHandles;
    waitHandles.reserve(context.notifications.size() + 1);
    waitHandles.push_back(context.thread.wait_handle());
    for (const auto& notification : context.notifications)
    {
      waitHandles.push_back(notification.get());
    }

    while (true)
    {
      auto result = WaitForMultipleObjects(static_cast<DWORD>(waitHandles.size()), waitHandles.data(), FALSE, INFINITE);
      if (result == WAIT_OBJECT_0) break;

      if (result >= WAIT_OBJECT_0 + 1 && result < WAIT_OBJECT_0 + waitHandles.size())
      {
        auto index = result - WAIT_OBJECT_0 - 1;
        _events.raise(directory_changed, this, context.directories[index]);

        if (!FindNextChangeNotification(waitHandles[index + 1])) break;
      }
      else
      {
        break;
      }
    }
  }
}
#endif
