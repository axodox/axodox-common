#pragma once
#include "common_includes.h"

namespace Axodox::Threading
{
  typedef std::chrono::duration<uint32_t, std::milli> event_timeout;

  class AXODOX_COMMON_API reset_event
  {
  public:
    void set();
    void reset();

    bool wait(std::chrono::steady_clock::duration timeout = {});

    explicit operator bool() const;

  protected:
    reset_event(bool auto_reset, bool signal);

  private:
    std::mutex _mutex;
    std::condition_variable _condition;
    
    bool _auto_reset;
    bool _signal;
  };

  struct AXODOX_COMMON_API manual_reset_event : public reset_event
  {
    manual_reset_event(bool signal = false);
  };

  struct AXODOX_COMMON_API auto_reset_event : public reset_event
  {
    auto_reset_event(bool signal = false);
  };
}