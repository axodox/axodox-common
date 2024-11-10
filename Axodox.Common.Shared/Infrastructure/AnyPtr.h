#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  template<typename T>
  void delete_value(void* value)
  {
    delete static_cast<T*>(value);
  }

  class any_ptr
  {
  public:
    any_ptr() = default;

    any_ptr(const any_ptr&) = delete;
    any_ptr& operator=(const any_ptr&) = delete;

    any_ptr(any_ptr&& other) noexcept
    {
      *this = std::move(other);
    }

    any_ptr& operator=(any_ptr&& other) noexcept
    {
      reset();

      std::swap(_data, other._data);
      std::swap(_deleter, other._deleter);
      std::swap(_type, other._type);
    }

    template<typename T>
    T* get() const noexcept
    {
      if (typeid(T) != _type) return nullptr;
      return reinterpret_cast<T*>(_data);
    }

    template<typename T>
    T* get_or_create()
    {
      auto result = get<T>();
      if (result) return result;

      return create<T>();
    }

    template<typename T, typename... TArgs>
    T* create(TArgs&&... args)
    {
      reset();

      _data = new T(std::forward<TArgs>(args)...);
      _deleter = &delete_value<T>;
      _type = typeid(T);

      return reinterpret_cast<T*>(_data);
    }

    void reset()
    {
      if (!_data) return;
      
      _deleter(_data);
      _data = nullptr;
      _type = typeid(void);
    }

    explicit operator bool() const
    {
      return _data != nullptr;
    }

    ~any_ptr()
    {
      reset();
    }

  private:
    void* _data = nullptr;
    void (*_deleter)(void*) = nullptr;
    std::type_index _type = typeid(void);
  };
}