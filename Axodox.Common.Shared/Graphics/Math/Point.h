#pragma once
#include "common_includes.h"

namespace Axodox::Graphics
{
  struct AXODOX_COMMON_API Point
  {
    int32_t X, Y;

    static const Point Zero;
  };
}