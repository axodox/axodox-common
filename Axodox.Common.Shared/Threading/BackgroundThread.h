#pragma once
#ifdef PLATFORM_WINDOWS
#include "Infrastructure/Events.h"
#include "Infrastructure/Logger.h"

namespace Axodox::Threading
{
  class AXODOX_COMMON_API background_thread
  {
    inline static const Infrastructure::logger _logger{ "background_thread" };

  public:
    background_thread() noexcept;
    explicit background_thread(const Infrastructure::event_handler<>& action, const std::string_view name = "background thread");
    ~background_thread() noexcept;

    background_thread(background_thread&& other) noexcept;
    const background_thread& operator =(background_thread&& other) noexcept;

    background_thread(const background_thread&) = delete;
    const background_thread& operator =(const background_thread&) = delete;

    bool is_running() const noexcept;
    bool is_exiting() const noexcept;

    void wait() const noexcept;

    void* wait_handle() const noexcept;

    explicit operator bool() const noexcept;
    void reset();

  private:
    struct context;

    std::unique_ptr<context> _context;

    static unsigned long __stdcall worker(void* argument) noexcept;
  };
}
#endif