#include "common_includes.h"
#include "Logger.h"
#include "Threading/BlockingCollection.h"
#include "Threading/LifetimeExecutor.h"
#include "Threading/Parallel.h"

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

  thread logging_thread;
  blocking_collection<log_entry> log_queue;

  void logging_worker();

  void initialize_logger()
  {
    logging_thread = thread(&logging_worker);
  }

  void shutdown_logger()
  {
    log_queue.complete();
    if (logging_thread.joinable()) logging_thread.join();
  }

  void logging_worker()
  {
#ifdef PLATFORM_UWP
    using namespace winrt;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Diagnostics;

    FileLoggingSession loggingSession{ L"Axodox-Logger" };
    unordered_map<string, ILoggingChannel> loggingChannels;
#endif

    log_entry entry;
    while (log_queue.try_get(entry))
    {
#ifdef PLATFORM_UWP
      auto& channel = loggingChannels[entry.channel];
      if (!channel)
      {
        channel = LoggingChannel(to_hstring(entry.channel));
        loggingSession.AddLoggingChannel(channel);
      }

      channel.LogMessage(to_hstring(entry.text), static_cast<LoggingLevel>(entry.severity));
#endif

#ifdef PLATFORM_WINDOWS 
      log_to_debug(entry);
      log_to_console(entry);
#else
      log_to_console(entry);
#endif
    }

#ifdef PLATFORM_UWP
    loggingSession.CloseAndSaveToFileAsync().get();
#endif
  }

  lifetime_executor<initialize_logger, shutdown_logger> logging_executor;
}

namespace Axodox::Infrastructure
{
  constexpr logger::logger(std::string_view channel) :
    _channel(channel)
  { }

  void logger::log(log_severity severity, std::string_view text) const
  {
    if (severity < ::severity) return;

    log_queue.add(log_entry{
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
}