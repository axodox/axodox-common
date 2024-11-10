#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  template<typename T>
  auto as_unique(const T& value)
  {
    return std::unique_ptr<T>{ const_cast<T*>(&value) };
  }
}