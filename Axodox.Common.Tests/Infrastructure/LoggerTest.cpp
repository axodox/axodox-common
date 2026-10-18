#include "common_includes.h"
#include "Include/Axodox.Infrastructure.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Axodox::Infrastructure;
using namespace std;
using namespace std::chrono_literals;
using namespace std::filesystem;

namespace
{
  //The worker writes the file asynchronously, so the tests poll for its effects
  template<typename predicate_t>
  bool eventually(predicate_t&& predicate)
  {
    for (auto i = 0; i < 500; i++)
    {
      if (predicate()) return true;
      this_thread::sleep_for(10ms);
    }

    return false;
  }

  string read_file(const path& file)
  {
    stringstream content;
    content << ifstream(file).rdbuf();
    return content.str();
  }
}

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
      Assert::IsTrue(logger::file() == file);

      logger{ "LoggerTests" }.log(log_severity::warning, "written to the file");

      string text;
      auto isWritten = eventually([&] {
        text = read_file(file);
        return text.find("WARN LoggerTests: written to the file") != string::npos;
        });

      logger::file({});
      Assert::IsTrue(logger::file().empty());

      //The worker closes the file asynchronously too, until then it cannot be removed
      eventually([&] { error_code error; remove_all(file.parent_path(), error); return !error; });

      Assert::IsTrue(isWritten, to_wstring(text).c_str());
    }
  };
}
