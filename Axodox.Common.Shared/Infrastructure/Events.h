#pragma once
#include "Threading/AwaitablePtr.h"
#include "Threading/Events.h"

namespace Axodox::Infrastructure
{
  template <typename... TArgs>
  class event_handler : public std::function<void(TArgs...)>
  {
  public:
    template<typename TInstance, typename TReturn = void>
    event_handler(TInstance* instance, TReturn(TInstance::* handler)(TArgs...)) :
      std::function<void(TArgs...)>([=](TArgs&&... args) { (instance->*handler)(std::forward<TArgs>(args)...); })
    {
    }

    using std::function<void(TArgs...)>::function;
  };

  class event_mutex
  {
  private:
    std::optional<std::recursive_mutex> _mutex;
    size_t _lockCount = 0;

  public:
    event_mutex(bool isSingleThreaded = false)
    {
      if (!isSingleThreaded) _mutex.emplace();
    }

    void lock()
    {
      if (_mutex) _mutex->lock();
      _lockCount++;
    }

    void unlock()
    {
      _lockCount--;
      if (_mutex) _mutex->unlock();
    }

    size_t lock_count() const
    {
      return _lockCount;
    }
  };

  class event_handler_collection_base
  {
  public:
    using token_t = uint32_t;

    virtual bool remove(token_t token) noexcept = 0;

    virtual ~event_handler_collection_base() noexcept = default;
  };

  class [[nodiscard]] event_subscription
  {
  public:
    event_subscription() = default;

    event_subscription(const std::shared_ptr<event_handler_collection_base>& eventHandlerCollection, event_handler_collection_base::token_t token) noexcept :
      _handlers(eventHandlerCollection),
      _token(token)
    { }

    event_subscription(event_subscription&& other) noexcept
    {
      *this = std::move(other);
    }

    event_subscription& operator=(event_subscription&& other) noexcept
    {
      reset();

      _handlers = std::move(other._handlers);
      _token = other._token;

      other._token = {};
      return *this;
    }

    event_subscription(const event_subscription&) = delete;
    event_subscription& operator=(const event_subscription&) = delete;

    ~event_subscription() noexcept
    {
      reset();
    }

    operator bool() const noexcept
    {
      return !_handlers.expired();
    }

    void reset()
    {
      auto eventHandlerCollection = _handlers.lock();
      if (eventHandlerCollection)
      {
        eventHandlerCollection->remove(_token);
        _handlers.reset();

        _token = {};
      }
    }

  private:
    std::weak_ptr<event_handler_collection_base> _handlers;
    event_handler_collection_base::token_t _token;
  };

  template<typename... TArgs>
  class event_handler_collection : public event_handler_collection_base
  {
  public:
    using handler_t = event_handler<TArgs...>;

    event_handler_collection(bool isSingleThreaded = false) :
      _nextToken(1),
      _mutex(isSingleThreaded)
    {
    };

    token_t add(handler_t&& handler) noexcept
    {
      std::lock_guard lock{ _mutex };

      while (_handlers.contains(_nextToken) || _handlersToAdd.contains(_nextToken))
      {
        _nextToken++;
      }

      if (_mutex.lock_count() == 1)
      {
        _handlers.emplace(_nextToken, std::move(handler));
      }
      else
      {
        _handlersToAdd.emplace(_nextToken, std::move(handler));
      }

      return _nextToken++;
    }

    virtual bool remove(token_t token) noexcept override
    {
      std::lock_guard lock{ _mutex };

      if (!_handlers.contains(token)) return false;

      if (_mutex.lock_count() == 1)
      {
        _handlers.erase(token);
      }
      else
      {
        _handlersToRemove.emplace(token);
      }

      return true;
    }

    void invoke(TArgs... args)
    {
      std::lock_guard lock{ _mutex };
      for (auto& [id, handler] : _handlers)
      {
        handler(std::forward<TArgs>(args)...);
      }

      if (_mutex.lock_count() == 1)
      {
        for (auto& [id, handler] : _handlersToAdd)
        {
          _handlers[id] = std::move(handler);
        }
        _handlersToAdd.clear();

        for (auto id : _handlersToRemove)
        {
          _handlers.erase(id);
        }
        _handlersToRemove.clear();
      }
    }

  private:
    event_mutex _mutex;
    std::unordered_map<token_t, handler_t> _handlers;
    std::map<token_t, event_handler<TArgs...>> _handlersToAdd;
    std::set<token_t> _handlersToRemove;
    token_t _nextToken = {};
  };

  struct no_revoke_t {};
  inline constexpr no_revoke_t no_revoke{};

  template<typename... TArgs>
  class event_publisher;

  class event_owner
  {
  public:
    event_owner() noexcept :
      _id(_nextId++)
    {
    }

    event_owner& operator=(event_owner&& other) noexcept
    {
      _id = other._id;
      other._id = size_t{};
      return *this;
    }

    event_owner(event_owner&& other) noexcept
    {
      *this = std::move(other);
    }

    event_owner(const event_owner&) = delete;
    event_owner& operator=(const event_owner&) = delete;

    template<typename... TArgs, typename TEvent = event_publisher<TArgs...>>
    void raise(TEvent& event, TArgs... args) const;

    bool is_valid() const noexcept
    {
      return _id != size_t{};
    }

    operator size_t() const noexcept
    {
      return _id;
    }

  private:
    static inline std::atomic_size_t _nextId = 1;
    size_t _id;
  };

  template<typename... TArgs>
  class event_publisher
  {
    friend class event_owner;

  private:
    size_t _ownerId;
    std::shared_ptr<event_handler_collection<TArgs...>> _handlers;

  public:
    event_publisher(event_publisher&&) = default;
    event_publisher& operator=(event_publisher&&) = default;

    event_publisher(const event_publisher&) = delete;
    event_publisher& operator=(const event_publisher&) = delete;

    event_publisher(const event_owner& owner, bool isSingleThreaded = false) noexcept :
      _ownerId(owner),
      _handlers(std::make_shared<event_handler_collection<TArgs...>>(isSingleThreaded))
    { }

    event_subscription subscribe(event_handler<TArgs...>&& handler) noexcept
    {
      return { _handlers, _handlers->add(std::move(handler)) };
    }

    void subscribe(no_revoke_t, event_handler<TArgs...>&& handler) noexcept
    {
      _handlers->add(std::move(handler));
    }

    event_subscription operator()(event_handler<TArgs...>&& handler) noexcept
    {
      return subscribe(std::move(handler));
    }

    void operator()(no_revoke_t, event_handler<TArgs...>&& handler) noexcept
    {
      subscribe(no_revoke, std::move(handler));
    }

    void raise(const event_owner& owner, TArgs... args)
    {
      if (owner != _ownerId)
      {
        throw std::logic_error("The specified event owner does not own this event.");
      }

      _handlers->invoke(std::forward<TArgs>(args)...);
    }

    Threading::awaitable_ptr<std::tuple<TArgs...>> wait(std::chrono::steady_clock::duration timeout = {});

    ~event_publisher() noexcept
    {
      _handlers.reset();
    }
  };

  template<typename... TArgs>
  class event_awaiter
  {
  public:
    explicit event_awaiter(event_publisher<TArgs...>& publisher)
    {
      using namespace Threading;
      using namespace std;

      _firedSubscription = publisher.subscribe([&](TArgs&&... args) {
        _waitingEvent.wait();
        if (_isShuttingDown) return;

        manual_reset_event resultFreedEvent;
        _result = make_waitable<tuple<TArgs...>>(resultFreedEvent, forward<TArgs>(args)...);

        _firedEvent.set();
        resultFreedEvent.wait();
        });
    }

    ~event_awaiter()
    {
      reset();
    }

    void reset()
    {
      _isShuttingDown = true;
      _waitingEvent.set();
      _firedSubscription.reset();
      _result.reset();
    }

    Threading::awaitable_ptr<std::tuple<TArgs...>> wait(std::chrono::steady_clock::duration timeout = {})
    {
      _waitingEvent.set();
      _firedEvent.wait(timeout);

      return std::move(_result);
    }

  private:
    std::atomic_bool _isShuttingDown = false;
    event_subscription _firedSubscription;
    Threading::auto_reset_event _waitingEvent;
    Threading::auto_reset_event _firedEvent;
    Threading::awaitable_ptr<std::tuple<TArgs...>> _result;
  };

  template<typename... TArgs>
  Threading::awaitable_ptr<std::tuple<TArgs...>> event_publisher<TArgs...>::wait(std::chrono::steady_clock::duration timeout)
  {
    event_awaiter awaiter{ *this };
    return awaiter.wait(timeout);
  }

  template<typename... TArgs, typename TEvent>
  void event_owner::raise(TEvent& event, TArgs... args) const
  {
    event.raise(*this, std::forward<TArgs>(args)...);
  }
}