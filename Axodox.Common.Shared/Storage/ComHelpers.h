#pragma once
#ifdef PLATFORM_WINDOWS
#include "common_includes.h"

namespace Axodox::Storage
{
  AXODOX_COMMON_API winrt::com_ptr<IStream> to_stream(std::span<const uint8_t> buffer);
}
#endif