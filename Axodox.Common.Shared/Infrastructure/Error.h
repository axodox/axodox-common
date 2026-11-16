#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  //Returns the message of the exception being handled: what() of a std::exception, the message of a winrt::hresult_error, otherwise "Unexpected error"
  //Call it from a catch block
  AXODOX_COMMON_API std::string get_error_message();
}
