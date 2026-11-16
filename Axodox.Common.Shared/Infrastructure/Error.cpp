#include "common_includes.h"
#include "Error.h"

using namespace std;

namespace Axodox::Infrastructure
{
  std::string get_error_message()
  {
    try
    {
      throw;
    }
    catch (const std::exception& exception)
    {
      return exception.what();
    }
#ifdef PLATFORM_WINDOWS
    catch (const winrt::hresult_error& error)
    {
      return winrt::to_string(error.message());
    }
#endif
    catch (...)
    {
      return "Unexpected error";
    }
  }
}
