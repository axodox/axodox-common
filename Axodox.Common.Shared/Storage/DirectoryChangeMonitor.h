#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"
#include "Infrastructure/Events.h"

namespace Axodox::Storage
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

  class AXODOX_COMMON_API directory_change_monitor
  {
    Infrastructure::event_owner _events;

  public:
    directory_change_monitor(std::span<const std::filesystem::path> directories);

    ~directory_change_monitor();

    directory_change_monitor(const directory_change_monitor&) = delete;
    directory_change_monitor& operator=(const directory_change_monitor&) = delete;

    Infrastructure::event_publisher<directory_change_monitor*, std::filesystem::path> directory_changed;

  private:
    std::vector<std::filesystem::path> _directories;
    std::vector<find_change_notification_handle> _notifications;
    std::vector<HANDLE> _waitHandles;
    winrt::handle _exiting;
    winrt::handle _worker;

    static unsigned long __stdcall monitor_changes(void* context) noexcept;
  };
}
#endif
