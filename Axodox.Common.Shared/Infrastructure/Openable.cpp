#include "common_includes.h"
#include "Openable.h"

namespace Axodox::Infrastructure
{
  bool openable::is_open() const noexcept
  {
    return _is_open;
  }

  void openable::open()
  {
    if (_is_open.exchange(true)) return;

    //A failed open can be tried again
    try
    {
      on_opening();
    }
    catch (...)
    {
      _is_open = false;
      throw;
    }
  }

  void openable::mark_closed() noexcept
  {
    _is_open = false;
  }
}
