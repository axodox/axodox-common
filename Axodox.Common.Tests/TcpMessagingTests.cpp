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
  TEST_CLASS(TcpMessagingTests)
  {
    static constexpr auto _waitTimeout = 60s;

    memory_stream MakeTextMessage(const string& payload)
    {
      memory_stream stream;
      stream.write(payload);
      return stream;
    }

    string ReadTextMessage(span<const uint8_t> buffer)
    {
      array_stream stream{ buffer };
      return stream.read<string>();
    }

  public:
    bool WaitForClients(tcp_messaging_server& server, uint32_t expected)
    {
      auto deadline = chrono::steady_clock::now() + _waitTimeout;
      while (server.client_count() != expected && chrono::steady_clock::now() < deadline)
      {
        this_thread::sleep_for(10ms);
      }

      return server.client_count() >= expected;
    }

    TEST_METHOD(ClientConnectsToServer)
    {
      //Create server
      tcp_messaging_server server{ 0 };
      event_awaiter serverConnectedAwaiter{ server.client_connected };
      event_awaiter serverDisconnectedAwaiter{ server.client_disconnected };

      //Start server
      server.open();
      Assert::AreNotEqual(uint16_t(0), server.port(), L"server should bind to a real ephemeral port");

      //Create client
      auto client = make_unique<tcp_messaging_client>(socket_address_ipv4{ ip_address_v4::loopback, server.port() });
      event_awaiter clientConnectedAwaiter{ client->connected };

      //Start client
      client->open();

      //Client reports connection
      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client should report connection");

      //Server reports connection
      Assert::IsTrue(bool(serverConnectedAwaiter.wait(_waitTimeout)), L"server should observe the new client");

      Assert::AreEqual(1u, server.client_count());

      //Disconnect client
      client.reset();
      Assert::IsTrue(bool(serverDisconnectedAwaiter.wait(_waitTimeout)));
    }

    TEST_METHOD(ClientToServerMessageRoundtrip)
    {
      //Create server
      tcp_messaging_server server{ 0 };
      event_awaiter serverMessageReceivedAwaiter{ server.message_received };
      server.open();

      //Create client
      tcp_messaging_client client{ socket_address_ipv4{ ip_address_v4::loopback, server.port() } };
      event_awaiter clientConnectedAwaiter{ client.connected };
      event_awaiter clientMessageReceivedAwaiter{ client.message_received };
      client.open();

      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"Client did not connect in time");
      client.send_message(MakeTextMessage("hello server")).future.get();

      auto serverMessageReceivedEvent = serverMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(serverMessageReceivedEvent), L"server did not receive message");
      Assert::AreEqual(string("hello server"), ReadTextMessage(get<1>(*serverMessageReceivedEvent)));
    }

    TEST_METHOD(ServerBroadcastReachesClient)
    {
      //Create server
      tcp_messaging_server server{ 0 };
      event_awaiter serverConnectedAwaiter{ server.client_connected };
      server.open();

      //Create client
      tcp_messaging_client client{ socket_address_ipv4{ ip_address_v4::loopback, server.port() } };
      event_awaiter clientConnectedAwaiter{ client.connected };
      event_awaiter clientMessageReceivedAwaiter{ client.message_received };
      client.open();

      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"Client did not connect in time");
      Assert::IsTrue(bool(serverConnectedAwaiter.wait(_waitTimeout)), L"server did not observe Client");

      //Broadcast from server
      server.broadcast_message(MakeTextMessage("hello Client"));

      auto clientMessageReceivedEvent = clientMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(clientMessageReceivedEvent), L"Client did not receive broadcast_message");
      Assert::AreEqual(string("hello Client"), ReadTextMessage(get<1>(*clientMessageReceivedEvent)));
    }

    TEST_METHOD(BroadcastReachesMultipleClientsAndRespectsException)
    {
      //Create server
      tcp_messaging_server server{ 0 };
      server.open();

      //Create clients
      tcp_messaging_client client_a{ socket_address_ipv4{ ip_address_v4::loopback, server.port() } };
      event_awaiter clientAConnectedAwaiter{ client_a.connected };
      event_awaiter clientAMessageReceivedAwaiter{ client_a.message_received };
      client_a.open();

      tcp_messaging_client client_b{ socket_address_ipv4{ ip_address_v4::loopback, server.port() } };
      event_awaiter clientBConnectedAwaiter{ client_b.connected };
      event_awaiter clientBMessageReceivedAwaiter{ client_b.message_received };
      client_b.open();

      Assert::IsTrue(bool(clientAConnectedAwaiter.wait(_waitTimeout)), L"Client a did not connect in time");
      Assert::IsTrue(bool(clientBConnectedAwaiter.wait(_waitTimeout)), L"Client b did not connect in time");
      Assert::IsTrue(WaitForClients(server, 2u), L"server did not register two clients");

      //Broadcast from server
      server.broadcast_message(MakeTextMessage("payload"));

      auto clientAMessageReceivedEvent = clientAMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(clientAMessageReceivedEvent), L"Client a missed broadcast_message");
      Assert::AreEqual(string("payload"), ReadTextMessage(get<1>(*clientAMessageReceivedEvent)));

      auto clientBMessageReceivedEvent = clientBMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(clientBMessageReceivedEvent), L"Client b missed broadcast_message");
      Assert::AreEqual(string("payload"), ReadTextMessage(get<1>(*clientBMessageReceivedEvent)));
    }

    TEST_METHOD(ServerDetectsClientDisconnect)
    {
      //Create server
      tcp_messaging_server server{ 0 };
      event_awaiter serverConnectedAwaiter{ server.client_connected };
      event_awaiter serverDisconnectedAwaiter{ server.client_disconnected };
      server.open();

      //Create client
      auto client = make_unique<tcp_messaging_client>(socket_address_ipv4{ ip_address_v4::loopback, server.port() });
      event_awaiter clientConnectedAwaiter{ client->connected };
      client->open();

      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"Client did not connect in time");
      Assert::IsTrue(bool(serverConnectedAwaiter.wait(_waitTimeout)), L"server did not observe Client");

      //Disconnect client
      client.reset();

      Assert::IsTrue(bool(serverDisconnectedAwaiter.wait(_waitTimeout)), L"server should detect disconnect");
    }

    TEST_METHOD(MultipleClientsLifecycle)
    {
      //Create server
      tcp_messaging_server server{ 0 };
      event_awaiter serverConnectedAwaiter{ server.client_connected };
      event_awaiter serverDisconnectedAwaiter{ server.client_disconnected };
      event_awaiter serverMessageReceivedAwaiter{ server.message_received };
      server.open();

      //Create clients
      struct ClientRecord
      {
        string Label;
        unique_ptr<tcp_messaging_client> Client;
        unique_ptr<event_awaiter<messaging_client*, messaging_channel*>> ConnectedAwaiter;
        unique_ptr<event_awaiter<messaging_channel*, span<const uint8_t>>> MessageReceivedAwaiter;
      };

      vector<ClientRecord> records(3);
      records[0].Label = "A";
      records[1].Label = "B";
      records[2].Label = "C";

      for (auto& r : records)
      {
        r.Client = make_unique<tcp_messaging_client>(socket_address_ipv4{ ip_address_v4::loopback, server.port() });
        r.ConnectedAwaiter = make_unique<event_awaiter<messaging_client*, messaging_channel*>>(r.Client->connected);
        r.MessageReceivedAwaiter = make_unique<event_awaiter<messaging_channel*, span<const uint8_t>>>(r.Client->message_received);
        r.Client->open();
      }

      //Wait for all clients to connect
      for (auto& r : records)
      {
        Assert::IsTrue(bool(r.ConnectedAwaiter->wait(_waitTimeout)), L"Client did not connect in time");
      }

      //Server observes all three connections
      for (uint32_t i = 0; i < 3; i++)
      {
        Assert::IsTrue(bool(serverConnectedAwaiter.wait(_waitTimeout)), L"server did not observe Client connection");
      }
      Assert::IsTrue(WaitForClients(server, 3u), L"server should have 3 clients");

      //Each client sends a unique message
      for (auto& r : records)
      {
        Assert::IsTrue(r.Client->send_message(MakeTextMessage("from-" + r.Label)).future.get(), L"Client send_message should have succeeded");
      }

      //Server receives all three messages
      set<string> serverMessages;
      for (uint32_t i = 0; i < 3; i++)
      {
        auto serverMessageReceivedEvent = serverMessageReceivedAwaiter.wait(_waitTimeout);
        Assert::IsTrue(bool(serverMessageReceivedEvent), L"server did not receive message");
        serverMessages.insert(ReadTextMessage(get<1>(*serverMessageReceivedEvent)));
      }
      Assert::AreEqual(size_t(3), serverMessages.size(), L"all three distinct messages should arrive");
      Assert::IsTrue(serverMessages.count("from-A") == 1);
      Assert::IsTrue(serverMessages.count("from-B") == 1);
      Assert::IsTrue(serverMessages.count("from-C") == 1);

      //Server broadcasts to all
      server.broadcast_message(MakeTextMessage("broadcast"));
      for (auto& r : records)
      {
        auto clientMessageReceivedEvent = r.MessageReceivedAwaiter->wait(_waitTimeout);
        Assert::IsTrue(bool(clientMessageReceivedEvent), L"Client missed broadcast");
        Assert::AreEqual(string("broadcast"), ReadTextMessage(get<1>(*clientMessageReceivedEvent)));
      }

      //Disconnect in order: B, A, C
      auto find = [&](string_view label) -> ClientRecord* {
        for (auto& r : records) if (r.Label == label) return &r;
        return nullptr;
        };

      find("B")->Client.reset();
      Assert::IsTrue(bool(serverDisconnectedAwaiter.wait(_waitTimeout)), L"server should detect B disconnect");
      Assert::IsTrue(WaitForClients(server, 2u), L"after B disconnect, server should have 2 clients");

      find("A")->Client.reset();
      Assert::IsTrue(bool(serverDisconnectedAwaiter.wait(_waitTimeout)), L"server should detect A disconnect");
      Assert::IsTrue(WaitForClients(server, 1u), L"after A disconnect, server should have 1 Client");

      find("C")->Client.reset();
      Assert::IsTrue(bool(serverDisconnectedAwaiter.wait(_waitTimeout)), L"server should detect C disconnect");
      Assert::IsTrue(WaitForClients(server, 0u), L"after C disconnect, server should have 0 clients");
    }

    TEST_METHOD(ClientReconnectsToNewServer)
    {
      //Create server A on an ephemeral port
      auto server_a = make_unique<tcp_messaging_server>(0);
      event_awaiter serverAConnectedAwaiter{ server_a->client_connected };
      server_a->open();
      uint16_t shared_port = server_a->port();

      //Create client targeting that port — survives the server swap
      tcp_messaging_client client{ socket_address_ipv4{ ip_address_v4::loopback, shared_port } };
      event_awaiter clientConnectedAwaiter{ client.connected };
      event_awaiter clientDisconnectedAwaiter{ client.disconnected };
      client.open();

      //Client connects to server A
      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"Client should connect to server A");
      Assert::IsTrue(bool(serverAConnectedAwaiter.wait(_waitTimeout)), L"server A should see the Client");
      Assert::IsTrue(client.is_connected(), L"Client.is_connected() should be true while attached to A");

      //Tear down server A — client should observe disconnect and start retrying
      serverAConnectedAwaiter.reset();
      server_a.reset();
      Assert::IsTrue(bool(clientDisconnectedAwaiter.wait(_waitTimeout)), L"Client should observe disconnect when server A is gone");
      Assert::IsFalse(client.is_connected(), L"Client.is_connected() should be false while detached");

      //Create server B on the same port
      tcp_messaging_server server_b{ shared_port };
      event_awaiter serverBConnectedAwaiter{ server_b.client_connected };
      event_awaiter serverBMessageReceivedAwaiter{ server_b.message_received };
      server_b.open();

      //Client reconnects to server B
      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"Client should reconnect to server B");
      Assert::IsTrue(bool(serverBConnectedAwaiter.wait(_waitTimeout)), L"server B should see the Client");
      Assert::IsTrue(client.is_connected(), L"Client.is_connected() should be true after reconnect");

      //Verify the new connection is functional
      Assert::IsTrue(client.send_message(MakeTextMessage("hello again")).future.get(), L"send_message to server B should succeed");

      {
        auto serverBMessageReceivedEvent = serverBMessageReceivedAwaiter.wait(_waitTimeout);
        Assert::IsTrue(bool(serverBMessageReceivedEvent), L"server B should receive the message");
        Assert::AreEqual(string("hello again"), ReadTextMessage(get<1>(*serverBMessageReceivedEvent)));
      }
    }
  };
}
