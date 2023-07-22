#pragma once
#ifdef PLATFORM_WINDOWS
#include "Infrastructure/Events.h"
#include "Infrastructure/Logger.h"
#include "Threading/Events.h"

namespace Axodox::Threading
{
  class AXODOX_COMMON_API background_thread
  {
    inline static const Infrastructure::logger _logger{ "background_thread" };

  public:
    background_thread() noexcept;
    explicit background_thread(const Infrastructure::event_handler<>& action, const char* name = "background thread");
    ~background_thread() noexcept;
    
    background_thread(background_thread&& other) noexcept;
    const background_thread& operator =(background_thread&& other) noexcept;

    background_thread(const background_thread&) = delete;
    const background_thread& operator =(const background_thread&) = delete;

    bool is_running() const noexcept;
    bool is_exiting() const noexcept;
        
    void wait() const noexcept;

    explicit operator bool() const noexcept;
    void reset() noexcept;

  private:
    const char* _name;
    std::function<void()> _action;
    winrt::handle _worker;
    bool _isExiting;
    manual_reset_event _isReady;

    static unsigned long __stdcall worker(void* argument) noexcept;
  };
}
#endif