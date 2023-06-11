#include "pch.h"
#include "Parallel.h"

using namespace std;

#ifdef PLATFORM_WINDOWS
using namespace winrt;
#endif

namespace Axodox::Threading
{
#ifdef PLATFORM_WINDOWS
  void set_thread_name(std::string_view name)
  {
    check_hresult(SetThreadDescription(GetCurrentThread(), to_hstring(name).c_str()));
  }

  std::string get_thread_name()
  {
    wchar_t* name;
    check_hresult(GetThreadDescription(GetCurrentThread(), &name));
    auto result = to_string(name);
    LocalFree(name);
    return result;
  }
#endif

#ifdef PLATFORM_LINUX
  void set_thread_name(std::string_view name)
  {
    pthread_setname_np(pthread_self(), name);
  }

  std::string get_thread_name()
  {
    std::string thread_name('\0', 16);
    pthread_getname_np(pthread_self(), thread_name.data(), thread_name.length());
    thread_name.resize(strlen(thread_name.c_str()));
    return thread_name;
  }
#endif

  thread_name_context::thread_name_context(std::string_view name)
  {
    _name = get_thread_name();
    set_thread_name(name);
  }

  thread_name_context::~thread_name_context()
  {
    set_thread_name(_name);
  }
}
