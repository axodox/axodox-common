#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  //Wraps an object owned elsewhere into a shared_ptr which never deletes it, so it can be handed
  //to an API taking shared ownership - such as a dependency container - without transferring it.
  //The caller has to keep the object alive for as long as any copy of the pointer is in use.
  template<typename T>
  std::shared_ptr<T> not_shared(T& value)
  {
    return { &value, [](T*) {} };
  }

  template<typename T>
  std::shared_ptr<T> not_shared(T* value)
  {
    return { value, [](T*) {} };
  }
}
