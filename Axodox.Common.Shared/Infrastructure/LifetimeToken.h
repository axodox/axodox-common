#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  class [[nodiscard]] AXODOX_COMMON_API lifetime_token
  {
  public:
    lifetime_token() = default;
    lifetime_token(std::function<void()>&& callback);

    lifetime_token(const lifetime_token&) = delete;
    lifetime_token& operator=(const lifetime_token&) = delete;

    lifetime_token(lifetime_token&& other) noexcept;
    lifetime_token& operator=(lifetime_token&& other) noexcept;

    ~lifetime_token();

    explicit operator bool() const;

    void reset();

  private:
    std::function<void()> _callback;
  };
}