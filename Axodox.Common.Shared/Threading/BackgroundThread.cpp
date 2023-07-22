#include "pch.h"
#ifdef PLATFORM_WINDOWS
#include "BackgroundThread.h"
#include "Parallel.h"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace winrt;

namespace Axodox::Threading
{
  background_thread::background_thread() noexcept :
    _name(nullptr),
    _isExiting(false)
  { }

  background_thread::background_thread(const Infrastructure::event_handler<>& action, const char* name) :
    _name(name),
    _action(action),
    _isExiting(false)
  {
    _worker = handle(CreateThread(nullptr, 0u, &background_thread::worker, this, 0u, nullptr));
    _isReady.wait();
  }

  background_thread::~background_thread() noexcept
  {
    reset();
  }

  background_thread::background_thread(background_thread&& other) noexcept
  {
    *this = move(other);
  }

  const background_thread& background_thread::operator=(background_thread&& other) noexcept
  {
    reset();

    swap(_name, other._name);
    swap(_action, other._action);
    swap(_isExiting, other._isExiting);
    swap(_worker, other._worker);

    return *this;
  }

  bool background_thread::is_running() const noexcept
  {
    return _worker ? WaitForSingleObject(_worker.get(), 0u) != WAIT_OBJECT_0 : false;
  }

  bool background_thread::is_exiting() const noexcept
  {
    return _worker ? _isExiting : false;
  }

  void background_thread::wait() const noexcept
  {
    if (!_worker) return;

    if (GetThreadId(GetCurrentThread()) == GetThreadId(_worker.get())) return;
    WaitForSingleObject(_worker.get(), INFINITE);
  }

  background_thread::operator bool() const noexcept
  {
    return bool(_worker);
  }

  void background_thread::reset() noexcept
  {
    if (!_worker) return;

    _isExiting = true;
    WaitForSingleObject(_worker.get(), INFINITE);
    _name = nullptr;
    _worker.close();
  }

  unsigned long __stdcall background_thread::worker(void* argument) noexcept
  {
    string name;
    function<void()> action;
    {
      auto that = static_cast<background_thread*>(argument);
      name = that->_name;
      action = that->_action;

      that->_isReady.set();
    }
    set_thread_name(name);

    try
    {
      action();
    }
    catch(...)
    {
      _logger.log(log_severity::error, string("Thread failed: ") + name);
    }

    return 0u;
  }
}
#endif