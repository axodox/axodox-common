#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  class void_ptr
  {
    using copier = void* (*)(const void*);

    template<typename value_t>
      requires std::is_trivially_copyable_v<value_t>
    static void* copy_value(const void* value)
    {
      auto size = sizeof(value_t);
      auto result = malloc(size);
      memcpy(result, value, size);
      return result;
    }

  public:
    void_ptr() :
      _copy(nullptr),
      _value(nullptr)
    {
    }

    template<typename value_t>
    void_ptr(const value_t& value) :
      _copy(&copy_value<value_t>),
      _value(_copy(&value))
    {
    }

    void_ptr(const void_ptr& other) :
      _copy(other._copy),
      _value(_copy(other._value))
    {
    }

    void_ptr& operator=(const void_ptr& other)
    {
      reset();

      _copy = other._copy;
      _value = _copy(other._value);

      return *this;
    }

    void_ptr(void_ptr&& other)
    {
      _copy = other._copy;
      _value = other._value;

      other._copy = nullptr;
      other._value = nullptr;
    }

    void_ptr& operator=(void_ptr&& other)
    {
      reset();

      _copy = other._copy;
      _value = other._value;

      other._copy = nullptr;
      other._value = nullptr;

      return *this;
    }

    ~void_ptr()
    {
      reset();
    }

    void reset()
    {
      if (!_value) return;

      free(_value);
      _value = nullptr;

      _copy = nullptr;
    }

    void* get() const
    {
      return _value;
    }

  private:
    copier _copy;
    void* _value;
  };
}