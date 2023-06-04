#pragma once
#include "pch.h"

namespace Axodox::Infrastructure
{
  template<template<typename...> class, typename...>
  struct is_instantiation_of : public std::false_type {};

  template<template<typename...> class U, typename... T>
  struct is_instantiation_of<U, U<T...>> : public std::true_type {};

  template<typename T>
  concept trivially_copyable = std::is_trivially_copyable_v<T>;

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
}