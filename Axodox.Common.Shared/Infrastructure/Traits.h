#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  template<template<typename...> class, typename...>
  struct is_instantiation_of : public std::false_type {};

  template<template<typename...> class U, typename... T>
  struct is_instantiation_of<U, U<T...>> : public std::true_type {};

  template<template<typename...> class U, typename... T>
  const bool is_instantiation_of_v = is_instantiation_of<U, T...>::value;

  struct supports_new_test
  {
    template<typename T>
    static std::true_type test(decltype(new T())*);

    template<typename T>
    static std::false_type test(...);
  };

  template<class T>
  struct supports_new : decltype(supports_new_test::test<T>(nullptr))
  { };

  struct supports_equals_test
  {
    template<typename T>
    static std::true_type test_equals(decltype(&T::operator==));

    template<typename T>
    static std::true_type test_not_equals(decltype(&T::operator!=));

    template<typename T>
    static std::false_type test_equals(...);

    template<typename T>
    static std::false_type test_not_equals(...);
  };

  template<class T>
  struct supports_equals : decltype(supports_equals_test::test_equals<T>(nullptr))
  {};

  template<class T>
  struct supports_not_equals : decltype(supports_equals_test::test_not_equals<T>(nullptr))
  {};

  template<typename T>
  struct pointed
  {
    using type = void;
  };

  template<typename T>
    requires std::is_pointer_v<T>
  struct pointed<T>
  {
    using type = std::remove_const_t<std::remove_pointer_t<T>>;
  };

  template<typename T>
    requires requires{ typename T::element_type; }
  struct pointed<T>
  {
    using type = std::remove_const_t<typename T::element_type>;
  };

  template<typename T>
  using pointed_t = typename pointed<T>::type;
}