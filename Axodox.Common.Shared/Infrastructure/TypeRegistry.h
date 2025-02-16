#pragma once
#include "TypeKeySource.h"

namespace Axodox::Infrastructure
{
  template<typename TBase>
  struct type_registration_base
  {
    virtual std::unique_ptr<TBase> create_unique() const = 0;
    virtual std::shared_ptr<TBase> create_shared() const = 0;
  };

  template<typename TBase, typename TDerived>
    requires std::is_base_of_v<TBase, TDerived>
  struct type_registration : public type_registration_base<TBase>
  {
    virtual std::unique_ptr<TBase> create_unique() const override
    {
      return std::make_unique<TDerived>();
    }

    virtual std::shared_ptr<TBase> create_shared() const override
    {
      return std::make_shared<TDerived>();
    }
  };

  template<typename TBase>
  class type_registry
  {
    std::unordered_map<uint32_t, std::unique_ptr<type_registration_base<TBase>>> _registrations;

    template<typename TFirst, typename... TRest>
      requires std::is_base_of_v<TBase, TFirst>
    void add_types()
    {
      TFirst value{};
      uint32_t key = get_index(&value);
      _registrations[key] = std::make_unique<type_registration<TBase, TFirst>>();

      if constexpr (sizeof...(TRest) > 0)
      {
        add_types<TRest...>();
      }
    }

  public:
    using key_type = decltype(type_key_source<TBase>{}(declval<const TBase*>()));

    std::unique_ptr<TBase> create_unique(uint32_t key) const noexcept
    {
      auto it = _registrations.find(key);
      if (it != _registrations.end())
      {
        return it->second->create_unique();
      }
      else
      {
        return nullptr;
      }
    }

    std::shared_ptr<TBase> create_shared(uint32_t key) const noexcept
    {
      auto it = _registrations.find(key);
      if (it != _registrations.end())
      {
        return it->second->create_shared();
      }
      else
      {
        return nullptr;
      }
    }

    uint32_t get_index(const TBase* value) const
    {
      return uint32_t(type_key_source<TBase>{}(value));
    }

    auto get_key(const TBase* value) const
    {
      return type_key_source<TBase>{}(value);
    }

    template<typename... TDerived>
    static type_registry<TBase> create()
    {
      type_registry<TBase> result;
      result.add_types<TDerived...>();
      return result;
    }
  };


  template<typename T>
  concept has_derived_types = requires
  {
    static_cast<type_registry<T>&>(T::derived_types);
  };
}