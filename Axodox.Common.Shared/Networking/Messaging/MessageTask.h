#pragma once
#include "common_includes.h"

namespace Axodox::Networking
{
  class AXODOX_COMMON_API message_promise
  {
  public:
    message_promise() = default;
    explicit message_promise(std::vector<uint8_t>&& message);

    const std::vector<uint8_t>& message() const;

    void cancel();
    bool is_canceled() const;

    void finish(bool success);
    bool is_finished() const;
    bool is_succeeded() const;

    std::future<bool> get_future();

    ~message_promise();

  private:
    std::vector<uint8_t> _message;
    std::promise<bool> _promise;
    std::mutex _mutex;
    bool _isCanceled = false;
    bool _isFinished = false;
    bool _isSucceeded = false;
  };

  class AXODOX_COMMON_API message_task
  {
  public:
    std::future<bool> future;

    message_task();
    explicit message_task(const std::shared_ptr<message_promise>& messagePromise);

    void cancel();

  private:
    std::weak_ptr<message_promise> _messagePromise;
  };
}
