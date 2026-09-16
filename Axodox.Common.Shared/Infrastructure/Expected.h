#pragma once
#include "common_includes.h"

namespace Axodox::Infrastructure
{
  //A custom error type opts in by declaring its own format_error next to itself, found by ADL.
  inline const char* format_error(const std::string& error)
  {
    return error.c_str();
  }

  //std::formattable arrives in C++23, so ask std::formatter whether it accepts the type instead.
  template <typename error_t>
    requires requires(std::formatter<error_t, char> formatter, std::format_parse_context& context) { formatter.parse(context); }
  std::string format_error(const error_t& error)
  {
    return std::format("{}", error);
  }

  template <typename error_t = std::string>
  class unexpected
  {
    error_t _error;

  public:
    explicit unexpected(error_t&& value) requires std::movable<error_t> :
      _error(std::move(value))
    { }

    explicit unexpected(const error_t& value) requires std::copyable<error_t> :
      _error(value)
    { }

    const error_t& error() const& { return _error; }
    error_t&& error() && { return std::move(_error); }

    bool operator==(const unexpected& other) const = default;
  };

  unexpected(const char*) -> unexpected<std::string>;

  template <typename error_t>
  unexpected(error_t) -> unexpected<error_t>;

  //An alternate spelling for where unexpected is inconvenient: MSVC keeps the removed C++98
  //::unexpected() until C++23, so the bare name is ambiguous wherever <eh.h> is in scope and this
  //namespace is opened. Qualifying works too, this just avoids having to.
  template <typename error_t = std::string>
  using failure = unexpected<error_t>;

  template <typename... args_t>
  unexpected<std::string> format_unexpected(std::format_string<args_t...> format, args_t&&... args)
  {
    return unexpected<std::string>(std::format(format, std::forward<args_t>(args)...));
  }

  template <typename result_t = void, typename error_t = std::string>
  class expected
  {
    std::optional<result_t> _result;
    std::optional<error_t> _error;

  public:
    expected() = delete;

    expected(result_t&& value) requires std::movable<result_t> :
      _result(std::move(value))
    { }

    expected(const result_t& value) requires std::copyable<result_t> :
      _result(value)
    { }

    template <typename source_t>
      requires std::constructible_from<error_t, const source_t&>
    expected(const unexpected<source_t>& error) :
      _error(error.error())
    { }

    template <typename source_t>
      requires std::constructible_from<error_t, source_t&&>
    expected(unexpected<source_t>&& error) :
      _error(std::move(error).error())
    { }

    const result_t& result() const
    {
      if (!_result) throw std::logic_error("The expected holds an error, not a result.");
      return *_result;
    }

    const error_t& error() const
    {
      if (!_error) throw std::logic_error("The expected holds a result, not an error.");
      return *_error;
    }

    const result_t& operator*() const { return *_result; }
    const result_t* operator->() const { return &*_result; }

    bool has_value() const { return _result.has_value(); }

    explicit operator bool() const { return has_value(); }

    void check() const
    {
      if (_error) throw std::runtime_error(format_error(*_error));
    }

    result_t value_or(result_t fallback) const requires std::copyable<result_t>
    {
      return _result ? *_result : std::move(fallback);
    }

    bool operator==(const expected& other) const = default;

    bool operator==(const result_t& value) const requires std::equality_comparable<result_t>
    {
      return _result == value;
    }

    bool operator==(const unexpected<error_t>& error) const requires std::equality_comparable<error_t>
    {
      return _error == error.error();
    }
  };

  template <typename error_t>
  class expected<void, error_t>
  {
    std::optional<error_t> _error;

  public:
    expected() = default;

    template <typename source_t>
      requires std::constructible_from<error_t, const source_t&>
    expected(const unexpected<source_t>& error) :
      _error(error.error())
    { }

    template <typename source_t>
      requires std::constructible_from<error_t, source_t&&>
    expected(unexpected<source_t>&& error) :
      _error(std::move(error).error())
    { }

    const error_t& error() const
    {
      if (!_error) throw std::logic_error("The expected holds a result, not an error.");
      return *_error;
    }

    bool has_value() const { return !_error.has_value(); }

    explicit operator bool() const { return has_value(); }

    void check() const
    {
      if (_error) throw std::runtime_error(format_error(*_error));
    }

    bool operator==(const expected& other) const = default;

    bool operator==(const unexpected<error_t>& error) const requires std::equality_comparable<error_t>
    {
      return _error == error.error();
    }
  };
}
