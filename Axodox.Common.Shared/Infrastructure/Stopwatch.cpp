#include "common_includes.h"
#include "Stopwatch.h"

using namespace std;
using namespace std::chrono;

namespace Axodox::Infrastructure
{
  Stopwatch::Stopwatch(std::string_view label) :
    _label(label),
    _start(steady_clock::now())
  { }

  Stopwatch::~Stopwatch()
  {
    auto end = steady_clock::now();
    auto elapsed = duration_cast<duration<float, milli>>(end - _start).count();
    _logger.log(log_severity::debug, "{}: {:.2f} ms", _label, elapsed);
  }
}