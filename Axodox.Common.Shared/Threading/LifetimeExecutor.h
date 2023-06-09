#pragma once

namespace Axodox::Threading
{
  template<void (*FInitialize)(), void (*FShutdown)()>
  class lifetime_executor
  {
  public:
    lifetime_executor()
    {
      if constexpr (FInitialize != nullptr) FInitialize();
    }

    ~lifetime_executor()
    {
      if constexpr (FShutdown != nullptr) FShutdown();
    }
  };
}