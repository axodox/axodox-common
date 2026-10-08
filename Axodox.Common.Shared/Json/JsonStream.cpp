#include "common_includes.h"
#include "JsonStream.h"

using namespace std;

namespace Axodox::Json
{
  json_stream::json_stream(const json_serialization_options& options) :
    options(options),
    _stream(&_ownedStream)
  { }

  json_stream::json_stream(std::stringstream& stream, const json_serialization_options& options) :
    options(options),
    _stream(&stream)
  { }

  void json_stream::push_indentation()
  {
    _indentation++;
  }

  void json_stream::pop_indentation()
  {
    if (_indentation > 0) _indentation--;
  }

  std::string json_stream::to_string() const
  {
    return _stream->str();
  }

  void json_stream::write_indentation()
  {
    //Indentation is deferred until the next write, so pushes and pops after a line break still apply to it
    if (!_isLineStart) return;

    _isLineStart = false;
    *_stream << string(_indentation * options.indentation_depth, ' ');
  }
}
