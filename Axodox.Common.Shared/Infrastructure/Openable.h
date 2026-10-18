#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  class AXODOX_COMMON_API openable
  {
  public:
    bool is_open() const noexcept;
    void open();

    virtual ~openable() = default;

  protected:
    virtual void on_opening() = 0;

    //For types which can be opened again after they were closed.
    void mark_closed() noexcept;

  private:
    std::atomic_bool _is_open = false;
  };
}
