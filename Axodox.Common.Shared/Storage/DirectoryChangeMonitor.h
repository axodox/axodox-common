#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"
#include "Infrastructure/Events.h"

namespace Axodox::Storage
{
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
    struct context;

    std::unique_ptr<context> _context;

    void monitor_changes(context& ctx) noexcept;
  };
}
#endif
