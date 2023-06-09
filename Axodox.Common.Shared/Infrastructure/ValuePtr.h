#pragma once
#include "Traits.h"

namespace Axodox::Infrastructure
{
  template<typename TValue>
  class value_ptr
  {
    template <typename TActual>
    friend class value_ptr;
  private:
    TValue* _value = nullptr;
    std::function<void* (const void*)> _copyConstructor = nullptr;

    template <typename TActual>
    void InitializeCopyConstructor()
    {
      _copyConstructor = []([[maybe_unused]]const void* other) -> void* {
        if constexpr (std::is_copy_constructible<TActual>::value)
        {
          auto value = new TActual(*(TActual*)other);
          return value;
        }
        else
        {
          throw std::exception("Cannot copy value!");
        }
      };
    }
  public:
    typedef TValue element_type;

    value_ptr() noexcept
    {
    }

    value_ptr(nullptr_t) noexcept
    {
    }

    template <typename TActual>
    value_ptr(std::unique_ptr<TActual>&& other) noexcept
    {
      _value = other.release();
      InitializeCopyConstructor<TActual>();
    }

    template <typename TActual>
    explicit value_ptr(TActual* value) noexcept
    {
      _value = value;
      InitializeCopyConstructor<TActual>();
    }

    value_ptr(const value_ptr<TValue>& other) noexcept
    {
      *this = other;
    }

    template <typename TActual>
    value_ptr(const value_ptr<TActual>& other) noexcept
    {
      *this = other;
    }

    value_ptr(value_ptr<TValue>&& other) noexcept
    {
      *this = std::move(other);
    }

    template <typename TActual>
    value_ptr(value_ptr<TActual>&& other) noexcept
    {
      *this = std::move(other);
    }

    template <typename TActual>
    value_ptr<TValue>& operator=(const value_ptr<TActual>& other) noexcept
    {
      reset();
      if (other._value)
      {
        _value = (TValue*)other._copyConstructor(other._value);
        _copyConstructor = other._copyConstructor;
      }
      return *this;
    }

    value_ptr<TValue>& operator=(const value_ptr<TValue>& other) noexcept
    {
      reset();
      if (other._value)
      {
        _value = (TValue*)other._copyConstructor(other._value);
        _copyConstructor = other._copyConstructor;
      }
      return *this;
    }

    template <typename TActual>
    value_ptr<TValue>& operator=(value_ptr<TActual>&& other) noexcept
    {
      reset();
      _value = static_cast<TValue*>(other._value);
      _copyConstructor = other._copyConstructor;
      other._value = nullptr;
      other._copyConstructor = nullptr;
      return *this;
    }

    value_ptr<TValue>& operator=(value_ptr<TValue>&& other) noexcept
    {
      reset();
      _value = other._value;
      _copyConstructor = other._copyConstructor;
      other._value = nullptr;
      other._copyConstructor = nullptr;
      return *this;
    }

    TValue* get() const noexcept
    {
      return _value;
    }

    const TValue& operator*() const noexcept
    {
      return *_value;
    }

    TValue& operator*() noexcept
    {
      return *_value;
    }

    TValue* operator->() const noexcept
    {
      return _value;
    }

    TValue& operator[](size_t id) const
    {
      return _value[id];
    }

    TValue* release() noexcept
    {
      auto value = _value;
      _value = nullptr;
      _copyConstructor = nullptr;
      return value;
    }

    void reset() noexcept
    {
      if (_value)
      {
        if constexpr (std::is_array<TValue>::value)
        {
          delete[] _value;
        }
        else
        {
          delete _value;
        }
      }
      _value = nullptr;
      _copyConstructor = nullptr;
    }

    template <typename TActual>
    void swap(value_ptr<TActual>& other) noexcept
    {
      std::swap(_value, other._value);
      std::swap(_copyConstructor, other._copyConstructor);
    }

    void swap(value_ptr<TValue>& other) noexcept
    {
      std::swap(_value, other._value);
      std::swap(_copyConstructor, other._copyConstructor);
    }

    bool operator==(const nullptr_t&) const noexcept
    {
      return _value == nullptr;
    }

    bool operator!=(const nullptr_t&) const noexcept
    {
      return _value != nullptr;
    }

    template <typename TActual, typename = std::enable_if_t<Traits::supports_equals<TValue>::value>>
    bool operator==(const value_ptr<TActual>& other) const noexcept
    {
      if (!_value || !other._value) return _value == other._value;

      return *_value == *other._value;
    }

    template<typename = std::enable_if_t<Traits::supports_equals<TValue>::value>>
    bool operator==(const value_ptr<TValue>& other) const noexcept
    {
      if (!_value || !other._value) return _value == other._value;

      return *_value == *other._value;
    }

    template <typename TActual, typename = std::enable_if_t<Traits::supports_not_equals<TValue>::value>>
    bool operator!=(const value_ptr<TActual>& other) const noexcept
    {
      if (!_value || !other._value) return _value != other._value;

      return *_value != *other._value;
    }

    template<typename = std::enable_if_t<Traits::supports_not_equals<TValue>::value>>
    bool operator!=(const value_ptr<TValue>& other) const noexcept
    {
      if (!_value || !other._value) return _value != other._value;

      return *_value != *other._value;
    }

    template <typename TActual>
    bool operator<(const value_ptr<TActual>& other) const noexcept
    {
      return _value < other._value;
    }

    bool operator<(const value_ptr<TValue>& other) const noexcept
    {
      return _value < other._value;
    }

    explicit operator bool() const noexcept
    {
      return _value != nullptr;
    }

    explicit operator std::unique_ptr<TValue>() const noexcept
    {
      if (_value != nullptr)
      {
        return std::unique_ptr<TValue>((TValue*)_copyConstructor(_value));
      }
      else
      {
        return nullptr;
      }
    }

    explicit operator std::shared_ptr<TValue>() const noexcept
    {
      if (_value != nullptr)
      {
        return std::shared_ptr<TValue>((TValue*)_copyConstructor(_value));
      }
      else
      {
        return nullptr;
      }
    }

    ~value_ptr() noexcept
    {
      reset();
    }
  };

  template<typename TValue, typename... TArgs>
  value_ptr<TValue> make_value(TArgs... args)
  {
    return value_ptr<TValue>(new TValue(std::forward<TArgs>(args)...));
  }
}