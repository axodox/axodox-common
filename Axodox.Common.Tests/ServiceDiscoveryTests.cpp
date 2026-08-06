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

    TEST_METHOD(TestWildcardAnnouncedAddressResolvesToSender)
    {
      socket_address_ipv6 multicastGroup{ { 0xff02, 0, 0, 0, 0, 0, 0, 0xbee0 }, 5556 };
      service_provider provider{ multicastGroup };
      service_locator locator{ multicastGroup };
      event_awaiter serviceAwaiter{ locator.service_found };

      // Announcing on the wildcard host means "reachable on any interface", so the locator
      // has to substitute the address the response actually arrived from.
      provider.announce("wildcard", socket_address_ipv4{ ip_address_v4::any, 6001 });

      auto event = serviceAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(event), L"the wildcard announcement was not received");

      const auto& found = get<1>(*event);
      Assert::AreEqual(string("wildcard"), found.id);

      // The announced port survives, the wildcard host does not.
      Assert::AreEqual(uint16_t(6001), found.address.port(), L"the announced port was lost");
      Assert::IsFalse(found.address.is_any(), L"the wildcard host was not replaced");
    }
  };

  TEST_CLASS(SocketAddressVariantTests)
  {
  public:
    TEST_METHOD(TestIsAnyForWildcardAddresses)
    {
      // The wildcard address a socket binds to to accept on any interface.
      socket_address_variant v4{ socket_address_ipv4{ ip_address_v4::any, 6000 } };
      Assert::IsTrue(v4.is_any(), L"0.0.0.0 is not reported as any");

      socket_address_variant v6{ socket_address_ipv6{ ip_address_v6::any, 6000 } };
      Assert::IsTrue(v6.is_any(), L"[::] is not reported as any");

      // A port does not make a wildcard address specific.
      socket_address_variant portless{ socket_address_ipv4{ ip_address_v4::any, 0 } };
      Assert::IsTrue(portless.is_any(), L"0.0.0.0:0 is not reported as any");
    }

    TEST_METHOD(TestIsAnyForSpecificAddresses)
    {
      socket_address_variant loopbackV4{ socket_address_ipv4{ ip_address_v4::loopback, 6000 } };
      Assert::IsFalse(loopbackV4.is_any(), L"127.0.0.1 is reported as any");

      socket_address_variant loopbackV6{ socket_address_ipv6{ ip_address_v6::loopback, 6000 } };
      Assert::IsFalse(loopbackV6.is_any(), L"[::1] is reported as any");

      socket_address_variant routableV4{ socket_address_ipv4{ { 192, 168, 1, 10 }, 6000 } };
      Assert::IsFalse(routableV4.is_any(), L"192.168.1.10 is reported as any");
    }

    TEST_METHOD(TestPortSetterKeepsHost)
    {
      // Setting the port must not disturb the host or the address family.
      socket_address_variant v4{ socket_address_ipv4{ ip_address_v4::loopback, 6000 } };
      v4.port(7000);
      Assert::AreEqual(uint16_t(7000), v4.port(), L"the ipv4 port was not updated");
      Assert::IsTrue(v4.as<socket_address_ipv4>()->address() == ip_address_v4::loopback, L"the ipv4 host changed");
      Assert::AreEqual(int(address_family::inet4), int(v4.type()), L"the ipv4 family changed");

      socket_address_variant v6{ socket_address_ipv6{ ip_address_v6::loopback, 6000 } };
      v6.port(7000);
      Assert::AreEqual(uint16_t(7000), v6.port(), L"the ipv6 port was not updated");
      Assert::IsTrue(v6.as<socket_address_ipv6>()->address() == ip_address_v6::loopback, L"the ipv6 host changed");
      Assert::AreEqual(int(address_family::inet6), int(v6.type()), L"the ipv6 family changed");
    }

    TEST_METHOD(TestPortSetterOnUnspecifiedAddressIsHarmless)
    {
      // There is no port field to write, so this is a no-op rather than a crash.
      socket_address_variant unspecified;
      unspecified.port(7000);
      Assert::AreEqual(uint16_t(0), unspecified.port(), L"an unspecified address reported a port");
      Assert::IsFalse(bool(unspecified), L"an unspecified address became specified");
    }

    TEST_METHOD(TestPortSetterRoundTripsThroughCopy)
    {
      // as<T>() hands back a copy, so writing through it must not affect the original -
      // this is exactly why the setter exists.
      socket_address_variant original{ socket_address_ipv4{ ip_address_v4::loopback, 6000 } };

      auto copy = original.as<socket_address_ipv4>();
      copy->port(9999);
      Assert::AreEqual(uint16_t(6000), original.port(), L"writing through an as<T>() copy changed the original");

      original.port(9999);
      Assert::AreEqual(uint16_t(9999), original.port(), L"the setter did not update the original");
    }

    TEST_METHOD(TestIsAnyForUnspecifiedAddress)
    {
      // A default constructed variant carries no address family at all, so it is not an
      // address - and specifically not the wildcard address. Use operator bool to tell the
      // two apart: an unspecified address is empty, a wildcard address is not.
      socket_address_variant unspecified;
      Assert::IsFalse(bool(unspecified), L"a default constructed variant is not empty");
      Assert::IsFalse(unspecified.is_any(), L"an unspecified address is reported as any");

      socket_address_variant wildcard{ socket_address_ipv4{ ip_address_v4::any, 0 } };
      Assert::IsTrue(bool(wildcard), L"a wildcard address is empty");
      Assert::IsTrue(wildcard.is_any(), L"a wildcard address is not reported as any");
    }
  };
}