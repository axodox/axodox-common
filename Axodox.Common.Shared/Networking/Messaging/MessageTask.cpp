#include "common_includes.h"
#include "MessageTask.h"

using namespace std;

namespace Axodox::Networking
{
  message_promise::message_promise(vector<uint8_t>&& message) :
    _message(move(message))
  { }

  span<const uint8_t> message_promise::message() const
  {
    return _message;
  }

  void message_promise::cancel()
  {
    lock_guard lock{ _mutex };
    if (_is_finished) return;

    _is_finished = true;
    _is_canceled = true;
    _is_succeeded = false;
    _promise.set_value(false);
  }

  bool message_promise::is_canceled() const
  {
    return _is_canceled;
  }

  void message_promise::finish(bool success)
  {
    lock_guard lock{ _mutex };
    if (_is_finished) return;

    _is_finished = true;
    _is_succeeded = success;
    _promise.set_value(success);
  }

  bool message_promise::is_finished() const
  {
    return _is_finished;
  }

  bool message_promise::is_succeeded() const
  {
    return _is_succeeded;
  }

  future<bool> message_promise::get_future()
  {
    return _promise.get_future();
  }

  message_promise::~message_promise()
  {
    lock_guard lock{ _mutex };
    if (!_is_finished)
    {
      _promise.set_value(false);
    }
  }

  message_task::message_task()
  {
    promise<bool> promise;
    future = promise.get_future();
    promise.set_value(false);
  }

  message_task::message_task(const shared_ptr<message_promise>& messagePromise) :
    _message_promise(messagePromise),
    future(messagePromise->get_future())
  { }

  void message_task::cancel()
  {
    auto messagePromise = _message_promise.lock();
    if (messagePromise)
    {
      messagePromise->cancel();
    }
  }
}
