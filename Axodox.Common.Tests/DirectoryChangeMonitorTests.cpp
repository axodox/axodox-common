#include "common_includes.h"
#include "Include/Axodox.Storage.h"
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace std;
using namespace std::filesystem;
using namespace chrono_literals;

namespace Axodox::Common::Tests
{
  TEST_CLASS(DirectoryChangeMonitorTests)
  {
    static constexpr auto _waitTimeout = 5s;

    path MakeUniqueTempDir(const wstring& tag)
    {
      auto base = temp_directory_path() / (L"axodox_dcm_" + tag + L"_" + to_wstring(GetCurrentProcessId()) + L"_" + to_wstring(GetTickCount64()));
      create_directories(base);
      return base;
    }

  public:
    TEST_METHOD(TestRaisesEventWhenFileCreated)
    {
      auto directory = MakeUniqueTempDir(L"create");
      auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) { remove_all(directory); });

      array<path, 1> directories{ directory };
      directory_change_monitor monitor{ directories };
      event_awaiter awaiter{ monitor.directory_changed };

      write_file(directory / "trigger.txt", span<const uint8_t>{});

      auto event = awaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(event), L"directory_changed should fire when a file is created");
      Assert::AreEqual(directory.native(), get<1>(*event).native(), L"raised path should match the watched directory");
    }

    TEST_METHOD(TestReportsCorrectDirectoryAmongMultiple)
    {
      auto directoryA = MakeUniqueTempDir(L"multiA");
      auto directoryB = MakeUniqueTempDir(L"multiB");
      auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) { remove_all(directoryA); remove_all(directoryB); });

      array<path, 2> directories{ directoryA, directoryB };
      directory_change_monitor monitor{ directories };
      event_awaiter awaiter{ monitor.directory_changed };

      write_file(directoryB / "only-b.txt", span<const uint8_t>{});

      auto event = awaiter.wait(_waitTimeout);
      Assert::IsTrue(bool(event), L"directory_changed should fire for B");
      Assert::AreEqual(directoryB.native(), get<1>(*event).native(), L"event should report directory B, not A");
    }

    TEST_METHOD(TestContinuesReportingAfterFirstChange)
    {
      auto directory = MakeUniqueTempDir(L"repeat");
      auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) { remove_all(directory); });

      array<path, 1> directories{ directory };
      directory_change_monitor monitor{ directories };
      event_awaiter awaiter{ monitor.directory_changed };

      write_file(directory / "first.txt", span<const uint8_t>{});
      Assert::IsTrue(bool(awaiter.wait(_waitTimeout)), L"first change should fire");

      write_file(directory / "second.txt", span<const uint8_t>{});
      Assert::IsTrue(bool(awaiter.wait(_waitTimeout)), L"second change should fire — FindNextChangeNotification must rearm");
    }
    
    TEST_METHOD(TestRaisesEventWhenFileDeleted)
    {
      auto directory = MakeUniqueTempDir(L"repeat");
      auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) { remove_all(directory); });

      array<path, 1> directories{ directory };
      directory_change_monitor monitor{ directories };
      event_awaiter awaiter{ monitor.directory_changed };

      write_file(directory / "first.txt", span<const uint8_t>{});
      Assert::IsTrue(bool(awaiter.wait(_waitTimeout)), L"first change should fire");

      std::filesystem::remove(directory / "first.txt");
      Assert::IsTrue(bool(awaiter.wait(_waitTimeout)), L"second change (delete) should fire — FindNextChangeNotification must rearm");
    }
    
    TEST_METHOD(TestRaisesEventWhenFileChanged)
    {
      auto directory = MakeUniqueTempDir(L"repeat");
      auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) { remove_all(directory); });

      array<path, 1> directories{ directory };
      directory_change_monitor monitor{ directories };
      event_awaiter awaiter{ monitor.directory_changed };

      write_file(directory / "first.txt", span<const uint8_t>{});
      Assert::IsTrue(bool(awaiter.wait(_waitTimeout)), L"first change should fire");

      std::ofstream first(directory / "first.txt", std::ios_base::app);
      first << "line\n";
      first.close();
      Assert::IsTrue(bool(awaiter.wait(_waitTimeout)), L"second change (append) should fire — FindNextChangeNotification must rearm");
    }
    
    TEST_METHOD(TestRaisesEventWhenFileRenamed)
    {
      auto directory = MakeUniqueTempDir(L"repeat");
      auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) { remove_all(directory); });

      array<path, 1> directories{ directory };
      directory_change_monitor monitor{ directories };
      event_awaiter awaiter{ monitor.directory_changed };

      write_file(directory / "first.txt", span<const uint8_t>{});
      Assert::IsTrue(bool(awaiter.wait(_waitTimeout)), L"first change should fire");

      std::filesystem::rename(directory / "first.txt", directory / "first_renamed.txt");
      Assert::IsTrue(bool(awaiter.wait(_waitTimeout)), L"second change (rename) should fire — FindNextChangeNotification must rearm");
    }
    
    TEST_METHOD(TestRaisesEventWhenDirectoryCreated)
    {
      auto directory = MakeUniqueTempDir(L"repeat");
      auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) { remove_all(directory); });

      array<path, 1> directories{ directory };
      directory_change_monitor monitor{ directories };
      event_awaiter awaiter{ monitor.directory_changed };

      std::filesystem::create_directory(directory/"first");
      Assert::IsFalse(bool(awaiter.wait(_waitTimeout)), L"directory creation should not fire");
    }

    TEST_METHOD(TestRaisesEventWhenFileInSubDirectoryCreated)
    {
      auto directory = MakeUniqueTempDir(L"repeat");
      auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) { remove_all(directory); });

      array<path, 1> directories{ directory };
      directory_change_monitor monitor{ directories };
      event_awaiter awaiter{ monitor.directory_changed };

      std::filesystem::create_directory(directory/"first");
      
      write_file(directory / "first/first.txt", span<const uint8_t>{});
      Assert::IsTrue(bool(awaiter.wait(_waitTimeout)), L"file creation in directory should fire");
    }
  };
}
