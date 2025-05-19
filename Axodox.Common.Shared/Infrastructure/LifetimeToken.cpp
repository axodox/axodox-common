#include "common_includes.h"
#include "LifetimeToken.h"

using namespace std;

namespace Axodox::Infrastructure
{
  lifetime_token::lifetime_token(std::function<void()>&& callback) :
    _callback(callback)
  { }

  lifetime_token::lifetime_token(lifetime_token&& other) noexcept
  {
    *this = move(other);
  }

  lifetime_token& lifetime_token::operator=(lifetime_token&& other) noexcept
  {
    reset();

    _callback = move(other._callback);
    return *this;
  }

  lifetime_token::~lifetime_token()
  {
    reset();
  }

  lifetime_token::operator bool() const
  {
    return bool(_callback);
  }

  void lifetime_token::reset()
  {
    if (!_callback) return;

    _callback();
    _callback = nullptr;
  }
}