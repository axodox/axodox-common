#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "Environment.h"

namespace Axodox::Infrastructure
{
  std::string get_environment_variable(std::string_view name)
  {
    char value[_MAX_ENV];
    if (GetEnvironmentVariableA(name.data(), value, _MAX_ENV))
    {
      char expandedValue[_MAX_ENV];
      ExpandEnvironmentStringsA(value, expandedValue, _MAX_ENV);
      return expandedValue;
    }
    else
    {
      return "";
    }
  }
}
#endif