#pragma once
#include "Infrastructure/Events.h"

namespace Axodox::Threading
{
  class cancellation_token
  {
    Infrastructure::event_owner _events;

  public:
    static const cancellation_token empty;

    Infrastructure::event_publisher<cancellation_token*> cancelled{ _events };

    cancellation_token() = default;

    cancellation_token(cancellation_token&&) = default;
    cancellation_token& operator=(cancellation_token&&) = default;

    cancellation_token(const cancellation_token&) = delete;
    cancellation_token& operator=(const cancellation_token&) = delete;

    void cancel()
    {
      _isCancelled = true;
    }

    bool is_cancelled() const
    {
      return _isCancelled;
    }

  private:
    bool _isCancelled = false;
  };

  const cancellation_token cancellation_token::empty = {};
}