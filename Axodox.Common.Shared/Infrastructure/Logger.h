#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  enum class log_severity
  {
    debug,
    information,
    warning,
    error,
    fatal
  };

  class AXODOX_COMMON_API logger
  {
  public:
    logger() = default;
    explicit constexpr logger(std::string_view channel);

    void log(log_severity severity, std::string_view text) const;

    void log(log_severity severity, std::wstring_view text) const;

    template<typename... TArgs>
    void log(log_severity severity, const std::format_string<TArgs...> format, TArgs&&... args) const
    {
      auto text = std::format(format, std::forward<TArgs>(args)...);
      log(severity, text);
    }

    template<typename... TArgs>
    void log(log_severity severity, const std::wformat_string<TArgs...> format, TArgs&&... args) const
    {
      auto text = std::format(format, std::forward<TArgs>(args)...);
      log(severity, text);
    }

  private:
    std::string_view _channel;
  };
}