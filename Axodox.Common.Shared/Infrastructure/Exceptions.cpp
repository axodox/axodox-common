#include "common_includes.h"
#include "Exceptions.h"

using namespace std;

namespace Axodox::Infrastructure
{
  std::string get_exception_message(const std::exception_ptr& exception)
  {
    if (!exception) return "No exception.";

    try
    {
      rethrow_exception(exception);
    }
    catch (const std::exception& error)
    {
      return error.what();
    }
#ifdef PLATFORM_WINDOWS
    catch (const winrt::hresult_error& error)
    {
      return winrt::to_string(error.message());
    }
#endif
    catch (...)
    {
      return "Unknown exception.";
    }
  }
}
