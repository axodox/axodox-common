#include "common_includes.h"
#include "JsonString.h"

using namespace Axodox::Infrastructure;

namespace Axodox::Json
{
  bool json_string::is_default() const
  {
    return value.empty();
  }

  void json_string::to_string(json_stream& stream) const
  {
    stream.write("\"");
    for (auto character : value)
    {
      switch (character)
      {
      case '"':
        stream.write("\\\"");
        break;
      case '\\':
        stream.write("\\\\");
        break;
      case '\r':
        stream.write("\\r");
        break;
      case '\n':
        stream.write("\\n");
        break;
      case '\t':
        stream.write("\\t");
        break;
      case '\0':
        stream.write("\"");
        return;
      default:
        stream.write(character);
        break;
      }
    }

    stream.write("\"");
  }

  Infrastructure::value_ptr<json_string> json_string::from_string(std::string_view& text)
  {
    if (text.empty()) return nullptr;

    std::stringstream result;
    bool isFirst = true;
    bool isEscaping = false;
    const char* end = nullptr;
    for (auto& character : text)
    {
      if (isFirst)
      {
        if (character != '"') return nullptr;
        isFirst = false;
      }
      else if (isEscaping)
      {
        isEscaping = false;
        switch (character)
        {
        case '0':
          result << '\0';
          break;
        case 'r':
          result << '\r';
          break;
        case 'n':
          result << '\n';
          break;
        case 't':
          result << '\t';
          break;
        case '"':
          result << '"';
          break;
        default:
          result << character;
          break;
        }
      }
      else if (character == '\\')
      {
        isEscaping = true;
      }
      else if (character == '"')
      {
        end = &character;
        break;
      }
      else
      {
        result << character;
      }
    }

    if (!end) return nullptr;
    text = text.substr(size_t(end - text.data()) + 1);

    return make_value<json_string>(result.str());
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