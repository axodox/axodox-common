#include "common_includes.h"
#include "Include/Axodox.Storage.h"
#include "Include/Axodox.Networking.h"
#include "Include/Axodox.Security.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace Axodox::Networking;
using namespace Axodox::Security;
using namespace Axodox::Storage;
using namespace Axodox::Threading;
using namespace std;
using namespace chrono_literals;

namespace Axodox::Common::Tests
{
  TEST_CLASS(NamedPipeMessagingTests)
  {
    static constexpr auto _waitTimeout = 5s;

    static string MakePipeName()
    {
      static atomic_uint32_t counter = 0;
      return format("axodox-common-tests-{}-{}", GetCurrentProcessId(), counter++);
    }

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

    bool WaitForClients(named_pipe_messaging_server& server, uint32_t expected)
    {
      auto deadline = chrono::steady_clock::now() + _waitTimeout;
      while (server.client_count() != expected && chrono::steady_clock::now() < deadline)
      {
        this_thread::sleep_for(10ms);
      }

      return server.client_count() == expected;
    }

  public:
    TEST_METHOD(TestClientConnectsToServer)
    {
      //Create server
      named_pipe_messaging_server server{ MakePipeName() };
      event_awaiter serverConnectedAwaiter{ server.client_connected };
      event_awaiter serverDisconnectedAwaiter{ server.client_disconnected };
      server.open();

      //Create client
      auto client = make_unique<named_pipe_messaging_client>(server.name());
      event_awaiter clientConnectedAwaiter{ client->connected };
      client->open();

      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client should report connection");
      Assert::IsTrue(bool(serverConnectedAwaiter.wait(_waitTimeout)), L"server should observe the new client");
      Assert::IsTrue(WaitForClients(server, 1u));

      //Disconnect client
      client.reset();
      Assert::IsTrue(bool(serverDisconnectedAwaiter.wait(_waitTimeout)), L"server should detect the disconnect");
      Assert::IsTrue(WaitForClients(server, 0u));
    }

    TEST_METHOD(TestClientConnectsOnceTheServerStarts)
    {
      //The client keeps retrying while there is no server yet
      auto name = MakePipeName();
      named_pipe_messaging_client client{ name };
      event_awaiter clientConnectedAwaiter{ client.connected };
      client.open();

      named_pipe_messaging_server server{ name };
      server.open();

      //The channel is not open yet while connected is raised, and the call site can reach the pipe specific members
      auto clientConnectedEvent = clientConnectedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(clientConnectedEvent), L"client did not connect once the server started");
      auto channel = static_cast<named_pipe_messaging_channel*>(get<1>(*clientConnectedEvent));
      Assert::AreEqual(uint32_t(GetCurrentProcessId()), channel->remote_peer().process_id);
      clientConnectedEvent.reset();

      Assert::IsTrue(WaitForClients(server, 1u));
    }

    TEST_METHOD(TestMessagesRoundtripInBothDirections)
    {
      //Connect
      named_pipe_messaging_server server{ MakePipeName() };
      event_awaiter serverConnectedAwaiter{ server.client_connected };
      event_awaiter serverMessageReceivedAwaiter{ server.message_received };
      server.open();

      named_pipe_messaging_client client{ server.name() };
      event_awaiter clientConnectedAwaiter{ client.connected };
      event_awaiter clientMessageReceivedAwaiter{ client.message_received };
      client.open();

      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client did not connect in time");
      auto serverConnectedEvent = serverConnectedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(serverConnectedEvent), L"server did not observe the client");

      //The server opens the channel after the connected event, so the awaited event must be released first
      auto serverChannel = get<1>(*serverConnectedEvent);
      serverConnectedEvent.reset();

      //Client to server
      Assert::IsTrue(client.send_message(MakeTextMessage("hello server")).future.get(), L"sending to the server should succeed");
      auto serverMessageReceivedEvent = serverMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(serverMessageReceivedEvent), L"server did not receive the message");
      Assert::AreEqual(string("hello server"), ReadTextMessage(get<1>(*serverMessageReceivedEvent)));

      //Server to client
      Assert::IsTrue(serverChannel->send_message(MakeTextMessage("hello client")).future.get(), L"sending to the client should succeed");
      auto clientMessageReceivedEvent = clientMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(clientMessageReceivedEvent), L"client did not receive the message");
      Assert::AreEqual(string("hello client"), ReadTextMessage(get<1>(*clientMessageReceivedEvent)));
    }

    TEST_METHOD(TestLargeMessageArrivesWhole)
    {
      named_pipe_messaging_server server{ MakePipeName() };
      event_awaiter serverMessageReceivedAwaiter{ server.message_received };
      server.open();

      named_pipe_messaging_client client{ server.name() };
      event_awaiter clientConnectedAwaiter{ client.connected };
      client.open();
      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client did not connect in time");

      //Larger than the read buffer and the pipe buffers, so it is read in several parts
      vector<uint8_t> message(1024 * 1024 + 7);
      for (size_t i = 0; i < message.size(); i++) message[i] = uint8_t(i * 31);

      auto expected = message;
      Assert::IsTrue(client.send_message(move(message)).future.get(), L"sending should succeed");

      auto serverMessageReceivedEvent = serverMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(serverMessageReceivedEvent), L"server did not receive the message");

      auto received = get<1>(*serverMessageReceivedEvent);
      Assert::AreEqual(expected.size(), received.size());
      Assert::IsTrue(equal(expected.begin(), expected.end(), received.begin()), L"the message should arrive unchanged");
    }

    TEST_METHOD(TestMessagesKeepTheirOrder)
    {
      named_pipe_messaging_server server{ MakePipeName() };
      vector<string> received;
      mutex receivedMutex;
      auto subscription = server.message_received([&](messaging_channel*, span<const uint8_t> message) {
        lock_guard lock{ receivedMutex };
        received.push_back(ReadTextMessage(message));
        });
      server.open();

      named_pipe_messaging_client client{ server.name() };
      event_awaiter clientConnectedAwaiter{ client.connected };
      client.open();
      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client did not connect in time");

      vector<message_task> tasks;
      for (auto i = 0; i < 100; i++)
      {
        tasks.push_back(client.send_message(MakeTextMessage(to_string(i))));
      }
      for (auto& task : tasks) Assert::IsTrue(task.future.get());

      auto deadline = chrono::steady_clock::now() + _waitTimeout;
      while (chrono::steady_clock::now() < deadline)
      {
        {
          lock_guard lock{ receivedMutex };
          if (received.size() == 100) break;
        }
        this_thread::sleep_for(10ms);
      }

      lock_guard lock{ receivedMutex };
      Assert::AreEqual(size_t(100), received.size());
      for (auto i = 0; i < 100; i++) Assert::AreEqual(to_string(i), received[i]);
    }

    TEST_METHOD(TestPeerIdentityIsReported)
    {
      named_pipe_messaging_server server{ MakePipeName() };
      event_awaiter serverConnectedAwaiter{ server.client_connected };
      server.open();

      named_pipe_messaging_client client{ server.name() };
      event_awaiter clientConnectedAwaiter{ client.connected };
      client.open();

      auto clientConnectedEvent = clientConnectedAwaiter.wait(_waitTimeout);
      auto serverConnectedEvent = serverConnectedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(clientConnectedEvent) && bool(serverConnectedEvent), L"both ends should connect");

      DWORD sessionId = 0;
      ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);

      for (auto channel : { get<1>(*clientConnectedEvent), get<1>(*serverConnectedEvent) })
      {
        auto pipeChannel = static_cast<named_pipe_messaging_channel*>(channel);
        Assert::AreEqual(uint32_t(GetCurrentProcessId()), pipeChannel->remote_peer().process_id);
        Assert::AreEqual(uint32_t(sessionId), pipeChannel->remote_peer().session_id);
      }
    }

    TEST_METHOD(TestSecondServerWithSameNameFails)
    {
      named_pipe_messaging_server first{ MakePipeName() };
      first.open();

      named_pipe_messaging_server second{ first.name() };
      Assert::ExpectException<runtime_error>([&] { second.open(); }, L"a second server must not take over the name");
    }

    TEST_METHOD(TestMultipleClientsAreServed)
    {
      named_pipe_messaging_server server{ MakePipeName() };
      event_awaiter serverMessageReceivedAwaiter{ server.message_received };
      server.open();

      vector<unique_ptr<named_pipe_messaging_client>> clients;
      for (auto i = 0; i < 3; i++)
      {
        auto client = make_unique<named_pipe_messaging_client>(server.name());
        event_awaiter clientConnectedAwaiter{ client->connected };
        client->open();
        Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client did not connect in time");
        clients.push_back(move(client));
      }
      Assert::IsTrue(WaitForClients(server, 3u), L"server should have three clients");

      for (auto i = 0; i < 3; i++)
      {
        Assert::IsTrue(clients[i]->send_message(MakeTextMessage("from-" + to_string(i))).future.get());
      }

      set<string> messages;
      for (auto i = 0; i < 3; i++)
      {
        auto event = serverMessageReceivedAwaiter.wait(_waitTimeout);
        Assert::IsTrue(bool(event), L"server did not receive a message");
        messages.insert(ReadTextMessage(get<1>(*event)));
      }
      Assert::AreEqual(size_t(3), messages.size(), L"all three distinct messages should arrive");

      clients.clear();
      Assert::IsTrue(WaitForClients(server, 0u), L"server should drop all clients");
    }

    TEST_METHOD(TestClientReconnectsToNewServer)
    {
      auto name = MakePipeName();

      //Server A
      auto serverA = make_unique<named_pipe_messaging_server>(name);
      serverA->open();

      named_pipe_messaging_client client{ name };
      event_awaiter clientConnectedAwaiter{ client.connected };
      event_awaiter clientDisconnectedAwaiter{ client.disconnected };
      client.open();
      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client should connect to server A");

      //Server A goes away
      serverA.reset();
      Assert::IsTrue(bool(clientDisconnectedAwaiter.wait(_waitTimeout)), L"client should observe the disconnect");
      Assert::IsFalse(client.is_connected());

      //Server B on the same name
      named_pipe_messaging_server serverB{ name };
      event_awaiter serverBMessageReceivedAwaiter{ serverB.message_received };
      serverB.open();

      Assert::IsTrue(bool(clientConnectedAwaiter.wait(_waitTimeout)), L"client should reconnect to server B");
      Assert::IsTrue(client.send_message(MakeTextMessage("hello again")).future.get(), L"sending to server B should succeed");

      auto event = serverBMessageReceivedAwaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(event), L"server B should receive the message");
      Assert::AreEqual(string("hello again"), ReadTextMessage(get<1>(*event)));
    }

    TEST_METHOD(TestPipeAccessIsLimitedToCurrentUser)
    {
      //The current user's SID
      winrt::handle token;
      winrt::check_bool(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, token.put()));
      DWORD size = 0;
      GetTokenInformation(token.get(), TokenUser, nullptr, 0, &size);
      vector<uint8_t> tokenUser(size);
      winrt::check_bool(GetTokenInformation(token.get(), TokenUser, tokenUser.data(), size, &size));
      auto userSid = reinterpret_cast<TOKEN_USER*>(tokenUser.data())->User.Sid;

      //The access list has a single entry, allowing the current user, and nothing is inherited
      win32_user_security security;
      auto descriptor = security.attributes()->lpSecurityDescriptor;

      BOOL isPresent = FALSE, isDefaulted = FALSE;
      PACL dacl = nullptr;
      winrt::check_bool(GetSecurityDescriptorDacl(descriptor, &isPresent, &dacl, &isDefaulted));
      Assert::IsTrue(isPresent && dacl, L"the pipe must have an access list");
      Assert::AreEqual(WORD(1), dacl->AceCount);

      ACCESS_ALLOWED_ACE* ace = nullptr;
      winrt::check_bool(GetAce(dacl, 0, reinterpret_cast<void**>(&ace)));
      Assert::AreEqual(BYTE(ACCESS_ALLOWED_ACE_TYPE), ace->Header.AceType);
      Assert::IsTrue(EqualSid(&ace->SidStart, userSid) != FALSE, L"the entry must be for the current user");

      SECURITY_DESCRIPTOR_CONTROL control = 0;
      DWORD revision = 0;
      winrt::check_bool(GetSecurityDescriptorControl(descriptor, &control, &revision));
      Assert::IsTrue((control & SE_DACL_PROTECTED) != 0, L"the access list must not inherit entries");
    }

    TEST_METHOD(TestServerPipeIsOwnedByCurrentUser)
    {
      named_pipe_messaging_server server{ MakePipeName() };
      server.open();

      auto path = named_pipe_messaging_channel::pipe_path(server.name());
      winrt::file_handle pipe{ CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr) };
      Assert::IsTrue(bool(pipe), L"the pipe should open");
      Assert::IsTrue(is_owned_by_current_user(pipe.get()));
    }

    TEST_METHOD(TestClientWithoutServerStaysDisconnected)
    {
      named_pipe_messaging_client client{ MakePipeName() };
      event_awaiter clientConnectedAwaiter{ client.connected };
      client.open();

      Assert::IsFalse(bool(clientConnectedAwaiter.wait(500ms)), L"there is no server to connect to");
      Assert::IsFalse(client.is_connected());
    }
  };
}
