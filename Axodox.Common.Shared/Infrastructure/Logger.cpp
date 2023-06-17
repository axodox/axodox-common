#include "pch.h"
#include "Logger.h"
#include "Threading/BlockingCollection.h"
#include "Threading/LifetimeExecutor.h"
#include "Threading/Parallel.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;

namespace
{
  struct log_entry
  {
    log_severity severity;
    string channel;
    string text;
  };

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
      auto log = std::format("{} {}: {}\n", to_string(entry.severity), entry.channel, entry.text);

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
      OutputDebugStringA(log.c_str());
#endif

      printf(log.c_str());
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
    log_queue.add(log_entry{
      .severity = severity,
      .channel = string(_channel),
      .text = string(text)
      });
  }

  void logger::log(log_severity severity, std::wstring_view text) const
  {
    log(severity, winrt::to_string(text));
  }
}