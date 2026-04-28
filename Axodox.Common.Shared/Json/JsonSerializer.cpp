#include "common_includes.h"
#include "JsonSerializer.h"

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
}