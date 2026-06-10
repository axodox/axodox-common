#include "common_includes.h"
#include "JsonValue.h"
#include "JsonNull.h"
#include "JsonBoolean.h"
#include "JsonNumber.h"
#include "JsonString.h"
#include "JsonArray.h"
#include "JsonObject.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Json
{
  std::string json_value::to_string() const
  {
    stringstream stream;
    to_string(stream);
    return stream.str();
  }

  Infrastructure::value_ptr<json_value> json_value::from_string(std::string_view& text)
  {
    json_skip_whitespace(text);

    if (text.empty()) return nullptr;

    switch (text.front())
    {
    case 'n':
      return json_null::from_string(text);
    case 't':
    case 'f':
      return json_boolean::from_string(text);
    case '"':
      return json_string::from_string(text);
    case '[':
      return json_array::from_string(text);
    case '{':
      return json_object::from_string(text);
    default:
      return json_number::from_string(text);
    }
  }

  void json_skip_whitespace(std::string_view& text)
  {
    for (auto& character : text)
    {
      switch (character)
      {
      case ' ':
      case '\r':
      case '\n':
      case '\t':
        //Skip white space
        break;
      default:
        text = text.substr(size_t(&character - text.data()));
        return;
      }
    }
  }

  Infrastructure::value_ptr<json_value> json_serializer<Infrastructure::value_ptr<json_value>>::to_json(const Infrastructure::value_ptr<json_value>& value)
  {
    return value;
  }

  bool json_serializer<Infrastructure::value_ptr<json_value>>::from_json(const json_value* json, Infrastructure::value_ptr<json_value>& value)
  {
    if (json)
    {
      switch (json->type())
      {
      case json_type::boolean:
        value = make_value<json_boolean>(static_cast<const json_boolean*>(json)->value);
        break;
      case json_type::number:
        value = make_value<json_number>(static_cast<const json_number*>(json)->value);
        break;
      case json_type::string:
        value = make_value<json_string>(static_cast<const json_string*>(json)->value);
        break;
      case json_type::object:
        value = make_value<json_object>(static_cast<const json_object*>(json)->value);
        break;
      case json_type::array:
        value = make_value<json_array>(static_cast<const json_array*>(json)->value);
        break;
      default:
        value = make_value<json_null>();
        break;
      }
      return true;
    }
    else
    {
      return false;
    }
  }

  bool json_serializer<Infrastructure::value_ptr<json_value>>::is_default(const Infrastructure::value_ptr<json_value>& value)
  {
    if (!value) return true;

    switch (value->type())
    {
    case json_type::null:
      return true;
    case json_type::boolean:
      return json_serializer<bool>::is_default(static_cast<const json_boolean*>(value.get())->value);
    case json_type::number:
      return json_serializer<double>::is_default(static_cast<const json_number*>(value.get())->value);
    case json_type::string:
      return json_serializer<std::string>::is_default(static_cast<const json_string*>(value.get())->value);
    case json_type::array:
      return json_serializer<json_array>::is_default(*static_cast<const json_array*>(value.get()));
    case json_type::object:
      return json_serializer<json_object>::is_default(*static_cast<const json_object*>(value.get()));
    default:
      return false;
    }
  }
}