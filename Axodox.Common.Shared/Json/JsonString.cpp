#include "common_includes.h"
#include "JsonString.h"
#include <charconv>

using namespace Axodox::Infrastructure;

namespace
{
  bool try_parse_code_unit(std::string_view text, uint16_t& value)
  {
    if (text.size() < 4) return false;

    auto [end, error] = std::from_chars(text.data(), text.data() + 4, value, 16);
    return error == std::errc{} && end == text.data() + 4;
  }

  void write_utf8(std::string& result, char32_t codePoint)
  {
    if (codePoint < 0x80)
    {
      result += char(codePoint);
    }
    else if (codePoint < 0x800)
    {
      result += char(0xC0 | (codePoint >> 6));
      result += char(0x80 | (codePoint & 0x3F));
    }
    else if (codePoint < 0x10000)
    {
      result += char(0xE0 | (codePoint >> 12));
      result += char(0x80 | ((codePoint >> 6) & 0x3F));
      result += char(0x80 | (codePoint & 0x3F));
    }
    else
    {
      result += char(0xF0 | (codePoint >> 18));
      result += char(0x80 | ((codePoint >> 12) & 0x3F));
      result += char(0x80 | ((codePoint >> 6) & 0x3F));
      result += char(0x80 | (codePoint & 0x3F));
    }
  }

  void write_unicode_escape(std::string& result, char character)
  {
    result += std::format("\\u{:04x}", uint8_t(character));
  }

  //Reads the hex digits of a \u escape, position points to the 'u' and is left on the last character consumed
  bool read_unicode_escape(std::string_view text, size_t& position, std::string& result)
  {
    //Code points outside the basic plane are written as a pair of UTF-16 surrogates, an unpaired one is replaced
    uint16_t codeUnit;
    if (!try_parse_code_unit(text.substr(position + 1), codeUnit)) return false;
    position += 4;

    char32_t codePoint = codeUnit;
    if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF)
    {
      uint16_t lowSurrogate;
      if (text.substr(position + 1, 2) == "\\u" && try_parse_code_unit(text.substr(position + 3), lowSurrogate) && lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF)
      {
        codePoint = 0x10000 + ((char32_t(codeUnit) - 0xD800) << 10) + (lowSurrogate - 0xDC00);
        position += 6;
      }
      else
      {
        codePoint = 0xFFFD;
      }
    }
    else if (codeUnit >= 0xDC00 && codeUnit <= 0xDFFF)
    {
      codePoint = 0xFFFD;
    }

    write_utf8(result, codePoint);
    return true;
  }
}

namespace Axodox::Json
{
  bool json_string::is_default() const
  {
    return value.empty();
  }

  void json_string::to_string(json_stream& stream) const
  {
    stream.write("\"", json_escape_string(value), "\"");
  }

  std::string json_escape_string(std::string_view value)
  {
    std::string result;
    result.reserve(value.size());

    for (auto character : value)
    {
      switch (character)
      {
      case '"':
        result += "\\\"";
        break;
      case '\\':
        result += "\\\\";
        break;
      case '\r':
        result += "\\r";
        break;
      case '\n':
        result += "\\n";
        break;
      case '\t':
        result += "\\t";
        break;
      case '\b':
        result += "\\b";
        break;
      case '\f':
        result += "\\f";
        break;
      case '\0':
        return result;
      default:
        //Other control characters are not allowed in json strings as they are
        if (uint8_t(character) < 0x20)
        {
          write_unicode_escape(result, character);
        }
        else
        {
          result += character;
        }
        break;
      }
    }

    return result;
  }

  Infrastructure::value_ptr<json_string> json_string::from_string(std::string_view& text)
  {
    if (text.empty() || text[0] != '"') return nullptr;

    //Find the closing quote, skipping over escaped characters
    for (size_t position = 1; position < text.size(); position++)
    {
      auto character = text[position];
      if (character == '\\')
      {
        position++;
      }
      else if (character == '"')
      {
        auto result = json_unescape_string(text.substr(1, position - 1));
        if (!result) return nullptr;

        text = text.substr(position + 1);
        return make_value<json_string>(std::move(*result));
      }
    }

    return nullptr;
  }

  std::optional<std::string> json_unescape_string(std::string_view text)
  {
    std::string result;
    result.reserve(text.size());

    for (size_t position = 0; position < text.size(); position++)
    {
      auto character = text[position];
      if (character != '\\')
      {
        result += character;
        continue;
      }

      if (++position == text.size()) return std::nullopt;

      switch (text[position])
      {
      case '0':
        result += '\0';
        break;
      case 'b':
        result += '\b';
        break;
      case 'f':
        result += '\f';
        break;
      case 'r':
        result += '\r';
        break;
      case 'n':
        result += '\n';
        break;
      case 't':
        result += '\t';
        break;
      case 'u':
        if (!read_unicode_escape(text, position, result)) return std::nullopt;
        break;
      default:
        result += text[position];
        break;
      }
    }

    return result;
  }

  Infrastructure::value_ptr<json_value> json_serializer<std::string>::to_json(const std::string& value)
  {
    return make_value<json_string>(value);
  }

  bool json_serializer<std::string>::is_default(const std::string& value)
  {
    return value.empty();
  }

  bool json_serializer<std::string>::from_json(const json_value* json, std::string& value)
  {
    if (json && json->type() == json_type::string)
    {
      value = static_cast<const json_string*>(json)->value;
      return true;
    }
    else
    {
      return false;
    }
  }
}