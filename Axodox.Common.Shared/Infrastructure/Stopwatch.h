#pragma once
#include "common_includes.h"
#include "Logger.h"

namespace Axodox::Infrastructure
{
  class AXODOX_COMMON_API Stopwatch
  {
    inline static const logger _logger{ "Stopwatch" };

  public:
    Stopwatch(std::string_view label);
    ~Stopwatch();

  private:
    std::string_view _label;
    std::chrono::steady_clock::time_point _start;
  };
}