#include "common_includes.h"
#ifdef PLATFORM_WINDOWS
#include "BackgroundThread.h"
#include "Parallel.h"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace winrt;

namespace Axodox::Threading
{
  //Everything tied to the worker's lifetime lives here, heap-allocated. The worker reads its inputs through
  //this struct instead of off the background_thread, so the storage never moves - the object can be moved by
  //swapping just the owning pointer, with no race against the worker's startup read.
  struct background_thread::context
  {
    std::string name;
    std::function<void()> action;
    winrt::handle exiting_event;
    winrt::handle worker;
  };

  background_thread::background_thread() noexcept = default;

  background_thread::background_thread(const Infrastructure::event_handler<>& action, const std::string_view name)
  {
    auto context = make_unique<background_thread::context>();
    context->name = name;
    context->action = action;
    context->exiting_event.attach(CreateEvent(nullptr, TRUE, FALSE, nullptr));

    //Publish the context before starting the worker so it (and wait_handle()) can be read immediately
    _context = move(context);
    _context->worker.attach(CreateThread(nullptr, 0u, &background_thread::worker, _context.get(), 0u, nullptr));
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

    swap(_context, other._context);

    return *this;
  }

  bool background_thread::is_running() const noexcept
  {
    return _context ? WaitForSingleObject(_context->worker.get(), 0u) != WAIT_OBJECT_0 : false;
  }

  bool background_thread::is_exiting() const noexcept
  {
    return _context ? WaitForSingleObject(_context->exiting_event.get(), 0u) == WAIT_OBJECT_0 : false;
  }

  void background_thread::wait() const noexcept
  {
    if (!_context) return;

    if (GetThreadId(GetCurrentThread()) == GetThreadId(_context->worker.get())) return;
    WaitForSingleObject(_context->worker.get(), INFINITE);
  }

  void* background_thread::wait_handle() const noexcept
  {
    return _context ? _context->exiting_event.get() : nullptr;
  }

  background_thread::operator bool() const noexcept
  {
    return bool(_context);
  }

  void background_thread::reset()
  {
    if (!_context) return;

    if (GetThreadId(_context->worker.get()) == GetCurrentThreadId())
    {
      throw logic_error("Attempting to destroy the currently running thread.");
    }

    SetEvent(_context->exiting_event.get());
    WaitForSingleObject(_context->worker.get(), INFINITE);

    _context.reset();
  }

  unsigned long __stdcall background_thread::worker(void* argument) noexcept
  {
    auto context = static_cast<background_thread::context*>(argument);

    set_thread_name(context->name);

    try
    {
      context->action();
    }
    catch (...)
    {
      _logger.log(log_severity::error, string("Thread failed: ") + context->name);
    }

    return 0u;
  }
}
#endif