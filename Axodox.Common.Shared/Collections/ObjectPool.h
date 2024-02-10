#pragma once
#include "pch.h"

namespace Axodox::Collections
{
  template<typename T>
  class object_pool;

  template<typename T>
  class object_pool_handle
  {
    friend class object_pool<T>;

  public:
    T& operator*() noexcept
    {
      return _object;
    }

    T* operator->() noexcept
    {
      return &_object;
    };

    operator T& ()
    {
      return _object;
    }

    operator const T& () const
    {
      return _object;
    }

    explicit operator bool() const
    {
      return _owner != nullptr;
    }

    object_pool_handle() :
      _owner(nullptr),
      _object()
    { }

    object_pool_handle(const object_pool_handle&) = delete;
    object_pool_handle& operator=(const object_pool_handle&) = delete;

    object_pool_handle(object_pool_handle&& other)
    {
      *this = std::move(other);
    }

    object_pool_handle& operator=(object_pool_handle&& other)
    {
      reset();
      std::swap(_owner, other._owner);
      std::swap(_object, other._object);
      return *this;
    }

    ~object_pool_handle()
    {
      reset();
    }

    void reset();

  private:
    object_pool_handle(object_pool<T>* owner, T&& value) :
      _owner(owner),
      _object(std::move(value))
    {}

    object_pool<T>* _owner;
    T _object;
  };

  template<typename T>
  class object_pool
  {
    friend class object_pool_handle<T>;

  public:
    explicit object_pool(const std::function<T()>& factory = [] { return T{}; }) :
      _factory(factory)
    { }

    [[nodiscard]] object_pool_handle<T> borrow()
    {
      {
        std::lock_guard lock(_mutex);
        if (_objects.empty())
        {
          return { this, _factory() };
        }
      }

      {
        T object{ std::move(_objects.front()) };
        _objects.pop();
        return { this, std::move(object) };
      }
    }

  private:
    std::mutex _mutex;
    std::queue<T> _objects;
    std::function<T()> _factory;

    void reclaim(T&& value)
    {
      std::lock_guard lock(_mutex);
      _objects.emplace(std::move(value));
    }
  };

  template<typename T>
  void object_pool_handle<T>::reset()
  {
    if (_owner) _owner->reclaim(std::move(_object));
  }
}