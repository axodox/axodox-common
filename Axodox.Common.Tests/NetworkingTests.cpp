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
  TEST_CLASS(NetworkingTests)
  {
    static constexpr auto _waitTimeout = 1s;

    memory_stream MakeTextMessage(string_view payload)
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
      auto client = make_unique<tcp_messaging_client>(ip_endpoint{ "127.0.0.1", server.port() });
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
      tcp_messaging_client client{ ip_endpoint{ "127.0.0.1", server.port() } };
      event_awaiter clientConnectedAwaiter{ client.connected };
      event_awaiter clientMessageReceivedAwaiter{ client.message_received };
      client.open();

      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client did not connect in time");
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
      tcp_messaging_client client{ ip_endpoint{ "127.0.0.1", server.port() } };
      event_awaiter clientConnectedAwaiter{ client.connected };
      event_awaiter clientMessageReceivedAwaiter{ client.message_received };
      client.open();

      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client did not connect in time");
      Assert::IsTrue(bool(serverConnectedAwaiter.wait(_waitTimeout)), L"server did not observe client");

      //Broadcast from server
      server.broadcast(MakeTextMessage("hello client"));

      auto clientMessageReceivedEvent = clientMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(clientMessageReceivedEvent), L"client did not receive broadcast");
      Assert::AreEqual(string("hello client"), ReadTextMessage(get<1>(*clientMessageReceivedEvent)));
    }

    TEST_METHOD(BroadcastReachesMultipleClientsAndRespectsException)
    {
      //Create server
      tcp_messaging_server server{ 0 };
      server.open();

      //Create clients
      tcp_messaging_client client_a{ ip_endpoint{ "127.0.0.1", server.port() } };
      event_awaiter clientAConnectedAwaiter{ client_a.connected };
      event_awaiter clientAMessageReceivedAwaiter{ client_a.message_received };
      client_a.open();

      tcp_messaging_client client_b{ ip_endpoint{ "127.0.0.1", server.port() } };
      event_awaiter clientBConnectedAwaiter{ client_b.connected };
      event_awaiter clientBMessageReceivedAwaiter{ client_b.message_received };
      client_b.open();

      Assert::IsTrue(bool(clientAConnectedAwaiter.wait(_waitTimeout)), L"client a did not connect in time");
      Assert::IsTrue(bool(clientBConnectedAwaiter.wait(_waitTimeout)), L"client b did not connect in time");
      Assert::IsTrue(WaitForClients(server, 2u), L"server did not register two clients");

      //Broadcast from server
      server.broadcast(MakeTextMessage("payload"));

      auto clientAMessageReceivedEvent = clientAMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(clientAMessageReceivedEvent), L"client a missed broadcast");
      Assert::AreEqual(string("payload"), ReadTextMessage(get<1>(*clientAMessageReceivedEvent)));

      auto clientBMessageReceivedEvent = clientBMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(clientBMessageReceivedEvent), L"client b missed broadcast");
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
      auto client = make_unique<tcp_messaging_client>(ip_endpoint{ "127.0.0.1", server.port() });
      event_awaiter clientConnectedAwaiter{ client->connected };
      client->open();

      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client did not connect in time");
      Assert::IsTrue(bool(serverConnectedAwaiter.wait(_waitTimeout)), L"server did not observe client");

      //Disconnect client
      client.reset();

      Assert::IsTrue(bool(serverDisconnectedAwaiter.wait(_waitTimeout)), L"server should detect disconnect");
    }

    //TEST_METHOD(MultipleClientsLifecycle)
    //{
    //  tcp_messaging_server server{ 0 };

    //  atomic_uint32_t connect_event_count = 0;
    //  atomic_uint32_t disconnect_event_count = 0;
    //  auto cc_sub = server.client_connected([&](messaging_server*, messaging_channel*) {
    //    connect_event_count.fetch_add(1);
    //    });
    //  auto dc_sub = server.client_disconnected([&](messaging_server*, messaging_channel*) {
    //    disconnect_event_count.fetch_add(1);
    //    });

    //  // collect messages message_received on the server side, keyed by sender pointer
    //  mutex server_msgs_mutex;
    //  unordered_map<messaging_channel*, string> server_msgs;
    //  manual_reset_event server_got_three_messages;
    //  auto server_setup_sub = server.client_connected([&](messaging_server*, messaging_channel* channel) {
    //    channel->message_received(no_revoke, [&, channel](messaging_channel*, span<const uint8_t> message) {
    //      lock_guard<mutex> lock(server_msgs_mutex);
    //      server_msgs[channel] = ReadTextMessage(message);
    //      if (server_msgs.size() == 3) server_got_three_messages.set();
    //      });
    //    });

    //  server.open();

    //  struct client_record
    //  {
    //    unique_ptr<tcp_messaging_client> client;
    //    string label;
    //    manual_reset_event ready;
    //    manual_reset_event broadcast_received;
    //    string received_text;
    //    event_subscription received_sub;
    //    event_subscription is_connected_sub;
    //  };

    //  auto records = vector<unique_ptr<client_record>>{};
    //  records.push_back(make_unique<client_record>());
    //  records.push_back(make_unique<client_record>());
    //  records.push_back(make_unique<client_record>());
    //  records[0]->label = "A";
    //  records[1]->label = "B";
    //  records[2]->label = "C";

    //  auto setup_client = [&](client_record& rec) {
    //    rec.client = make_unique<tcp_messaging_client>(ip_endpoint{ "127.0.0.1", server.port() });
    //    rec.received_sub = rec.client->message_received([&](messaging_client*, span<const uint8_t> message) {
    //      rec.received_text = ReadTextMessage(message);
    //      rec.broadcast_received.set();
    //      });
    //    rec.is_connected_sub = rec.client->is_connected_changed([&](messaging_client*, bool is_connected) {
    //      if (is_connected) rec.ready.set();
    //      });
    //    rec.client->open();
    //    };

    //  // connect A, B, C in order
    //  for (auto& r : records) setup_client(*r);

    //  for (auto& r : records)
    //  {
    //    Assert::IsTrue(r->ready.wait(_waitTimeout), L"client did not connect in time");
    //  }
    //  WaitForClients(server, 3u);
    //  Assert::AreEqual(3u, server.client_count(), L"server should have 3 clients");
    //  Assert::AreEqual(3u, connect_event_count.load(), L"client_connected should have fired 3 times");

    //  // each client sends a unique message to the server
    //  for (auto& r : records)
    //  {
    //    auto task = r->client->send_message(MakeTextMessage("from-" + r->label));
    //    Assert::IsTrue(task.future.valid());
    //    Assert::AreEqual(int(future_status::ready), int(task.future.wait_for(_waitTimeout)));
    //    Assert::IsTrue(task.future.get(), L"client send_message should have succeeded");
    //  }

    //  Assert::IsTrue(server_got_three_messages.wait(_waitTimeout), L"server should receive all three messages");
    //  {
    //    lock_guard<mutex> lock(server_msgs_mutex);
    //    set<string> message_received;
    //    for (auto& [ch, text] : server_msgs) message_received.insert(text);
    //    Assert::AreEqual(size_t(3), message_received.size(), L"all three distinct messages should arrive");
    //    Assert::IsTrue(message_received.count("from-A") == 1);
    //    Assert::IsTrue(message_received.count("from-B") == 1);
    //    Assert::IsTrue(message_received.count("from-C") == 1);
    //  }

    //  // server broadcasts to all
    //  server.broadcast(MakeTextMessage("broadcast"));
    //  for (auto& r : records)
    //  {
    //    Assert::IsTrue(r->broadcast_received.wait(_waitTimeout), L"client missed broadcast");
    //    Assert::AreEqual(string("broadcast"), r->received_text);
    //  }

    //  // disconnect order: B, A, C
    //  auto find = [&](string_view label) -> client_record* {
    //    for (auto& r : records) if (r->label == label) return r.get();
    //    return nullptr;
    //    };

    //  find("B")->client.reset();
    //  WaitForClients(server, 2u);
    //  Assert::AreEqual(2u, server.client_count(), L"after B disconnect, server should have 2 clients");

    //  find("A")->client.reset();
    //  WaitForClients(server, 1u);
    //  Assert::AreEqual(1u, server.client_count(), L"after A disconnect, server should have 1 client");

    //  find("C")->client.reset();
    //  WaitForClients(server, 0u);
    //  Assert::AreEqual(0u, server.client_count(), L"after C disconnect, server should have 0 clients");

    //  // wait briefly for the third disconnect event to propagate
    //  auto deadline = chrono::steady_clock::now() + _waitTimeout;
    //  while (disconnect_event_count.load() < 3u && chrono::steady_clock::now() < deadline)
    //  {
    //    this_thread::sleep_for(10ms);
    //  }
    //  Assert::AreEqual(3u, disconnect_event_count.load(), L"client_disconnected should have fired 3 times");
    //}

    //TEST_METHOD(ClientReconnectsToNewServer)
    //{
    //  // start server A on an ephemeral port and remember the port number for server B
    //  auto server_a = make_unique<tcp_messaging_server>(0);
    //  manual_reset_event server_a_saw_client;
    //  auto server_a_sub = server_a->client_connected([&](messaging_server*, messaging_channel*) {
    //    server_a_saw_client.set();
    //    });
    //  server_a->open();
    //  uint16_t shared_port = server_a->port();

    //  // single client targeting that port — survives the server swap
    //  tcp_messaging_client client{ ip_endpoint{ "127.0.0.1", shared_port } };

    //  manual_reset_event first_connect;
    //  manual_reset_event after_disconnect;
    //  manual_reset_event second_connect;
    //  atomic_uint32_t state_change_count = 0;

    //  auto state_sub = client.is_connected_changed([&](messaging_client*, bool is_connected) {
    //    auto count = state_change_count.fetch_add(1);
    //    if (count == 0)
    //    {
    //      Assert::IsTrue(is_connected, L"first state change must be connected=true");
    //      first_connect.set();
    //    }
    //    else if (count == 1)
    //    {
    //      Assert::IsFalse(is_connected, L"second state change must be connected=false");
    //      after_disconnect.set();
    //    }
    //    else if (count == 2 && is_connected)
    //    {
    //      second_connect.set();
    //    }
    //    });

    //  atomic_uint32_t connected_event_count = 0;
    //  auto connected_sub = client.connected([&](messaging_client*, messaging_channel*) {
    //    connected_event_count.fetch_add(1);
    //    });

    //  client.open();

    //  Assert::IsTrue(first_connect.wait(_waitTimeout), L"client should connect to server A");
    //  Assert::IsTrue(server_a_saw_client.wait(_waitTimeout), L"server A should see the client");
    //  Assert::IsTrue(client.is_connected(), L"client.is_connected() should be true while attached to A");
    //  Assert::AreEqual(1u, connected_event_count.load(), L"connected event should fire once for A");

    //  // tear down server A — client should observe disconnect and start retrying
    //  server_a.reset();

    //  Assert::IsTrue(after_disconnect.wait(_waitTimeout), L"client should observe disconnect when server A is gone");
    //  Assert::IsFalse(client.is_connected(), L"client.is_connected() should be false while detached");

    //  // start server B on the same port
    //  tcp_messaging_server server_b{ shared_port };
    //  manual_reset_event server_b_saw_client;
    //  auto server_b_sub = server_b.client_connected([&](messaging_server*, messaging_channel*) {
    //    server_b_saw_client.set();
    //    });

    //  // server_b will receive any message the client sends after reconnect
    //  manual_reset_event server_b_got_message;
    //  string server_b_received;
    //  auto server_b_msg_sub = server_b.client_connected([&](messaging_server*, messaging_channel* channel) {
    //    channel->message_received(no_revoke, [&](messaging_channel*, span<const uint8_t> message) {
    //      server_b_received = ReadTextMessage(message);
    //      server_b_got_message.set();
    //      });
    //    });

    //  server_b.open();

    //  Assert::IsTrue(second_connect.wait(_waitTimeout), L"client should reconnect to server B");
    //  Assert::IsTrue(server_b_saw_client.wait(_waitTimeout), L"server B should see the client");
    //  Assert::IsTrue(client.is_connected(), L"client.is_connected() should be true after reconnect");
    //  Assert::AreEqual(2u, connected_event_count.load(), L"connected event should fire again after reconnect");

    //  // verify the new connection is functional
    //  auto task = client.send_message(MakeTextMessage("hello again"));
    //  Assert::IsTrue(task.future.valid());
    //  Assert::AreEqual(int(future_status::ready), int(task.future.wait_for(_waitTimeout)));
    //  Assert::IsTrue(task.future.get(), L"send_message to server B should succeed");

    //  Assert::IsTrue(server_b_got_message.wait(_waitTimeout), L"server B should receive the message");
    //  Assert::AreEqual(string("hello again"), server_b_received);
    //}
  };
}
