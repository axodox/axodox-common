#include "common_includes.h"
#include "UdpClient.h"
#include "Networking/Sockets/Addressing/SocketAddressV4.h"
#include "Networking/Sockets/Addressing/SocketAddressV6.h"

using namespace Axodox::Threading;
using namespace std;

namespace Axodox::Networking
{
  udp_client::udp_client(const udp_options& options) :
    udp_client(socket_address_ipv6{ ip_address_v6::any, 0 }, options)
  {
  }

  udp_client::udp_client(uint16_t port, const udp_options& options) :
    udp_client(socket_address_ipv6{ ip_address_v6::any, port }, options)
  {
  }

  udp_client::udp_client(const socket_address& address, const udp_options& options) :
    message_received(_events),
    _socket(address.type(), socket_type::datagram, ip_protocol::udp)
  {
    if (options.is_address_reused) _socket.set_option(SOL_SOCKET, SO_REUSEADDR, 1);
    if (options.is_broadcast) _socket.set_option(SOL_SOCKET, SO_BROADCAST, 1);

    _socket.bind(address);

    if (!holds_alternative<monostate>(options.multicast_group))
    {
      if (holds_alternative<ip_address_v4>(options.multicast_group))
      {
        ip_mreq multicastGroup{
          .imr_multiaddr = get<ip_address_v4>(options.multicast_group).to_in_addr(),
          .imr_interface = 0
        };
        _socket.set_option(IPPROTO_IP, IP_ADD_MEMBERSHIP, multicastGroup);
      }

      if (holds_alternative<ip_address_v6>(options.multicast_group))
      {
        ipv6_mreq multicastGroup{
          .ipv6mr_multiaddr = get<ip_address_v6>(options.multicast_group).to_in6_addr(),
          .ipv6mr_interface = 0
        };
        _socket.set_option(IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, multicastGroup);
      }
    }

    _receiver = make_unique<background_thread>([this] { receive(); }, "* UDP receiver - " + _socket.local_address().to_string());
  }

  udp_client::~udp_client()
  {
    _socket.reset();
    _receiver.reset();
  }

  void udp_client::connect(const socket_address& address)
  {
    _socket.connect(address);
  }

  void udp_client::send_message(std::span<const uint8_t> message)
  {
    _socket.send(message);
  }

  void udp_client::send_message(const udp_addressed_message& message)
  {
    _socket.send_to(message.content, message.address);
  }

  const socket& udp_client::client() const
  {
    return _socket;
  }

  void udp_client::receive()
  {
    try
    {
      vector<uint8_t> buffer;
      buffer.resize(65536);

      while (!_receiver->is_exiting())
      {
        socket_address_variant address;

        auto length = _socket.receive_from(buffer, address);
        if (length == 0) break;

        udp_addressed_message message{
          .address = address,
          .content = span{ buffer.data(), length }
        };

        try
        {
          _events.raise(message_received, this, message);
        }
        catch (...)
        {
          //Ignore error
        }
      }
    }
    catch (...)
    {
      //Ignore error
    }
  }
}