#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  //The message of an exception, for logging it: what() for a std::exception, the message of a winrt::hresult_error, or a
  //placeholder for anything else. Defaults to the exception being handled, so it can be used in a catch (...) block.
  AXODOX_COMMON_API std::string get_exception_message(const std::exception_ptr& exception = std::current_exception());
}
