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

  private:
    bool _is_open = false;
  };
}
