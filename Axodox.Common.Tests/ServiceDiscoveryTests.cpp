#include "common_includes.h"
#include "Include/Axodox.Storage.h"
#include "Include/Axodox.Networking.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace Axodox::Networking;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace std;
using namespace chrono_literals;

namespace Axodox::Common::Tests
{
  TEST_CLASS(ServiceDiscoveryTests)
  {
    static constexpr auto _waitTimeout = 5s;

  public:
    TEST_METHOD(TestServiceDiscovery)
    {
      socket_address_ipv6 multicastGroup{ { 0xff02, 0, 0, 0, 0, 0, 0, 0xbeef }, 5555 };
      service_provider provider{ multicastGroup };
      service_locator locator{ multicastGroup };
      event_awaiter serviceAwaiter{ locator.service_found };

      provider.announce("test", socket_address_ipv4{ {127, 0, 0, 1}, 6000 });

      {
        auto event = serviceAwaiter.wait(_waitTimeout);
        Assert::IsTrue(bool(event));
        Assert::AreEqual(string("test"), get<1>(*event).id);
      }

      locator.locate_service("test");

      {
        auto event = serviceAwaiter.wait(_waitTimeout);
        Assert::IsTrue(bool(event));
        Assert::AreEqual(string("test"), get<1>(*event).id);
      }
    }
  };
}