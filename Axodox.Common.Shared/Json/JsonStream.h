#pragma once
#include "common_includes.h"

namespace Axodox::Json
{
  //Controls how json values are written to text.
  struct json_serialization_options
  {
    bool is_indented = false;
    size_t indentation_depth = 2;
    std::string_view line_ending = "\n";
  };

  //Text output for json values, which optionally breaks the output into indented lines.
  //When indentation is disabled, write_line() behaves the same as write().
  //It either owns its buffer, or writes into an existing stringstream, which must outlive it.
  class AXODOX_COMMON_API json_stream
  {
  public:
    json_serialization_options options;

    explicit json_stream(const json_serialization_options& options = {});
    explicit json_stream(std::stringstream& stream, const json_serialization_options& options = {});

    json_stream(const json_stream&) = delete;
    json_stream& operator=(const json_stream&) = delete;

    template<typename... args_t>
    void write(args_t&&... args)
    {
      write_indentation();
      (*_stream << ... << std::forward<args_t>(args));
    }

    template<typename... args_t>
    void write_line(args_t&&... args)
    {
      write(std::forward<args_t>(args)...);
      if (options.is_indented)
      {
        *_stream << options.line_ending;
        _isLineStart = true;
      }
    }

    void push_indentation();
    void pop_indentation();

    std::string to_string() const;

  private:
    std::stringstream _ownedStream;
    std::stringstream* _stream;
    size_t _indentation = 0;
    bool _isLineStart = false;

    void write_indentation();
  };
}
