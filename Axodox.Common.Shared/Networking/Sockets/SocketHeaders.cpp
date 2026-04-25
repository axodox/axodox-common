#include "common_includes.h"
#include "SocketHeaders.h"
#include "Threading/LifetimeExecutor.h"

namespace
{
  void initialize_winsock()
  {
    WSADATA data{};
    auto result = WSAStartup(MAKEWORD(2, 2), &data);
    if (result != 0)
    {
      throw std::runtime_error("Failed to initialize Winsock.");
    }
  }

  void shutdown_winsock()
  {
    WSACleanup();
  }

  Axodox::Threading::lifetime_executor<initialize_winsock, shutdown_winsock> winsock_executor;
}
