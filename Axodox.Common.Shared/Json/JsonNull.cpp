#include "common_includes.h"
#include "JsonNull.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Json
{
  json_type json_null::type() const
  {
    return json_type::null;
  }

  void json_null::to_string(std::stringstream& stream) const
  {
    stream << "null";
  }

  Infrastructure::value_ptr<json_null> json_null::from_string(std::string_view& text)
  {
    if (text.compare(0, 4, "null") == 0)
    {
      text = text.substr(4);
      return make_value<json_null>();
    }
    else
    {
      return nullptr;
    }
  }
}