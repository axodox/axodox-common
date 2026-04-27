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
    if (_is_open) return;
    _is_open = true;
    on_opening();
  }
}
