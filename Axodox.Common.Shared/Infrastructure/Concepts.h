#pragma once
#include "Traits.h"

namespace Axodox::Infrastructure
{
  //Forward declared, as ValuePtr.h relies on the concepts below.
  template<typename TValue>
  class value_ptr;

  //Abstract types are excluded before new T() is tried: MSVC (14.50) fails to convert lambdas returning
  //std::shared_ptr<T> to std::function once it has substituted new T() for an abstract T.
  template<typename T>
  concept supports_new = !std::is_abstract_v<T> && requires { new T(); };

  template<typename T>
  concept supports_equals = requires(const T& left, const T& right)
  {
    { left == right } -> std::convertible_to<bool>;
  };

  template<typename T>
  concept supports_not_equals = requires(const T& left, const T& right)
  {
    { left != right } -> std::convertible_to<bool>;
  };

  template<typename T>
  concept supports_to_string = requires (T value)
  {
    { value.to_string() } -> std::same_as<std::string>;
  };

  template<typename T>
  concept supports_from_string = requires (const std::string_view value)
  {
    { T::from_string(value) } -> std::same_as<std::optional<T>>;
  };

  template<typename T>
  concept supports_to_from_string = supports_to_string<T> && supports_from_string<T>;

  template<typename T>
  concept trivially_copyable = std::is_trivially_copyable_v<T>;

  template <typename TInstance, template <typename...> class TTemplate>
  concept instantiation_of = is_instantiation_of_v<TTemplate, TInstance>;

  template<typename T>
  concept pointer_type = std::is_pointer_v<T> || instantiation_of<T, std::unique_ptr> || instantiation_of<T, std::shared_ptr> || instantiation_of<T, value_ptr>;

  template<typename T>
  concept is_pointing = !std::is_same_v<pointed_t<T>, void>;
}