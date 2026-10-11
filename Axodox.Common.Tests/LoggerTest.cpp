#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;
using namespace std::chrono_literals;
using namespace std::filesystem;

namespace Axodox::Common::Tests
{
  TEST_CLASS(LoggerTests)
  {
  public:
    TEST_METHOD(TestLogIsAppendedToTheFile)
    {
      auto file = temp_directory_path() / format("axodox-logger-test-{}", GetCurrentProcessId()) / "test.log";
      remove_all(file.parent_path());
      logger::file(file);

      logger{ "LoggerTests" }.log(log_severity::warning, "written to the file");

      //The log is written on the logger's own thread
      string text;
      for (int attempt = 0; attempt < 100 && text.find("written to the file") == string::npos; attempt++)
      {
        this_thread::sleep_for(10ms);
        stringstream content;
        content << ifstream(file).rdbuf();
        text = content.str();
      }

      logger::file({});
      remove_all(file.parent_path());

      Assert::IsTrue(text.find("WARN LoggerTests: written to the file") != string::npos, to_wstring(text).c_str());
    }
  };
}
