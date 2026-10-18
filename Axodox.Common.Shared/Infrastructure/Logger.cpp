#include "common_includes.h"
#include "Logger.h"
#include "Threading/BlockingCollection.h"
#include "Threading/LifetimeExecutor.h"
#include "Threading/Parallel.h"
#include "Infrastructure/Win32.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;
using namespace std::chrono;

namespace
{
  log_severity severity = log_severity::information;

  struct log_entry
  {
    system_clock::time_point time;
    log_severity severity;
    string channel;
    string text;
  };

  //Queued along with the entries, so that the worker switches files in order with them
  struct log_file_change
  {
    filesystem::path path;
  };

  using log_message = variant<log_entry, log_file_change>;

  std::string_view to_string(log_severity severity)
  {
    switch (severity)
    {
    case log_severity::debug:
      return "DEBUG";
    case log_severity::information:
      return "INFO";
    case log_severity::warning:
      return "WARN";
    case log_severity::error:
      return "ERROR";
    case log_severity::fatal:
      return "FATAL";
    default:
      return "";
    }
  }

#ifdef PLATFORM_WINDOWS
  void log_to_console(const log_entry& entry)
  {
    auto console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!console) return;

    static const array<WORD, 5> severityColors =
    {
      FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
      FOREGROUND_BLUE | FOREGROUND_INTENSITY,
      FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
      FOREGROUND_RED | FOREGROUND_INTENSITY,
      FOREGROUND_RED | FOREGROUND_INTENSITY
    };

    auto time = zoned_time{ current_zone(), time_point_cast<seconds>(entry.time) };
    auto header = std::format("{:%T} {} {}: ", time, to_string(entry.severity), entry.channel);
    auto message = entry.text + "\n";

    CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo{};
    GetConsoleScreenBufferInfo(console, &screenBufferInfo);
    SetConsoleTextAttribute(console, severityColors[size_t(entry.severity)]);
    WriteConsoleA(console, header.data(), uint32_t(header.length()), nullptr, nullptr);
    SetConsoleTextAttribute(console, screenBufferInfo.wAttributes);
    WriteConsoleA(console, message.data(), uint32_t(message.length()), nullptr, nullptr);
  }

  void log_to_debug(const log_entry& entry)
  {
    auto message = std::format("{} {}: {}\n", to_string(entry.severity), entry.channel, entry.text);
    OutputDebugStringA(message.c_str());
  }
#else
  void log_to_console(const log_entry& entry)
  {
    auto time = zoned_time{ current_zone(), time_point_cast<seconds>(entry.time) };
    auto log = std::format("{:%T} {} {}: {}\n", time, to_string(entry.severity), entry.channel, entry.text);
    printf(log.c_str());
  }
#endif

  void set_log_file(ofstream& file, const filesystem::path& path)
  {
    file.close();
    if (path.empty()) return;

    error_code error;
    filesystem::create_directories(path.parent_path(), error);
    file.open(path, ios::app);
  }

  void log_to_file(ofstream& file, const log_entry& entry)
  {
    if (!file.is_open()) return;

    auto time = zoned_time{ current_zone(), time_point_cast<milliseconds>(entry.time) };
    file << std::format("{:%F %T} {} {}: {}\n", time, to_string(entry.severity), entry.channel, entry.text) << flush;
  }

  filesystem::path log_file_path;

  thread logging_thread;
  blocking_collection<log_message> log_queue;

  void logging_worker();
  void shutdown_logger();

  void initialize_logger()
  {
    logging_thread = thread(&logging_worker);

#if defined(PLATFORM_WINDOWS) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    //Process exit terminates the other threads, the worker too, before a DLL's static destructors run. The process's
    //exit handlers run earlier, after the executable's static destructors, so the worker can still finish there. The
    //module is pinned, as unloading it would leave the handler dangling.
    HMODULE module;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, reinterpret_cast<LPCWSTR>(&shutdown_logger), &module);
    _crt_atexit(&shutdown_logger);
#endif
  }

  //Writes the queued entries and stops the worker; entries logged later are dropped.
  void shutdown_logger()
  {
    log_queue.complete();
    if (logging_thread.joinable()) logging_thread.join();
  }

  void logging_worker()
  {
    thread_name_context threadNameContext{ "* logger" };

#ifdef PLATFORM_UWP
    using namespace winrt;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Diagnostics;

    FileLoggingSession loggingSession{ nullptr };
    if (has_package_identity())
    {
      loggingSession = FileLoggingSession{ std::format(L"Axodox-Logger-{}", GetCurrentProcessId()) };
    }
    unordered_map<string, ILoggingChannel> loggingChannels;
#endif

    ofstream logFile;

    log_message message;
    while (log_queue.try_get(message))
    {
      if (auto entry = get_if<log_entry>(&message))
      {
#ifdef PLATFORM_UWP
        if (loggingSession)
        {
          auto& channel = loggingChannels[entry->channel];
          if (!channel)
          {
            channel = LoggingChannel(to_hstring(entry->channel));
            loggingSession.AddLoggingChannel(channel);
          }

          channel.LogMessage(to_hstring(entry->text), static_cast<LoggingLevel>(entry->severity));
        }
#endif

#ifdef PLATFORM_WINDOWS
        log_to_debug(*entry);
        log_to_console(*entry);
#else
        log_to_console(*entry);
#endif
        log_to_file(logFile, *entry);
      }
      else
      {
        set_log_file(logFile, get<log_file_change>(message).path);
      }
    }

#ifdef PLATFORM_UWP
    if (loggingSession) loggingSession.CloseAndSaveToFileAsync().get();
#endif
  }

  lifetime_executor<initialize_logger, shutdown_logger> logging_executor;
}

namespace Axodox::Infrastructure
{
  void logger::log(log_severity severity, std::string_view text) const
  {
    if (severity < ::severity) return;

    log_queue.try_add(log_entry{
      .time = system_clock::now(),
      .severity = severity,
      .channel = string(_channel),
      .text = string(text)
      });
  }

  void logger::log(log_severity severity, std::wstring_view text) const
  {
    log(severity, winrt::to_string(text));
  }

  log_severity logger::severity()
  {
    return ::severity;
  }

  void logger::severity(log_severity value)
  {
    ::severity = value;
  }

  std::filesystem::path logger::file()
  {
    return log_file_path;
  }

  void logger::file(const std::filesystem::path& path)
  {
    log_file_path = path;
    log_queue.try_add(log_file_change{ path });
  }
}