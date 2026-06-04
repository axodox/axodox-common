#include "common_includes.h"
#include "JsonSerializer.h"
#include "Infrastructure/Text.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Json
{
  json_property_base::json_property_base(const char* name) :
    _name(name)
  { }

  const char* json_property_base::name() const
  {
    return _name;
  }

  std::vector<const json_property_base*> json_object_base::properties() const
  {
    vector<const json_property_base*> results;
    for (auto propertyOffset : _propertyOffsets)
    {
      auto property = (const json_property_base*)(intptr_t(this) + propertyOffset);
      results.push_back(property);
    }
    return results;
  }

  value_ptr<json_value> json_base64_converter::to_json(const std::vector<uint8_t>& value)
  {
    return make_value<json_string>(encode_base64(value));
  }

  bool json_base64_converter::from_json(const json_value* json, std::vector<uint8_t>& value)
  {
    if (!json || json->type() != json_type::string) return false;

    return try_decode_base64(static_cast<const json_string*>(json)->value, value);
  }
}