#include "common_includes.h"
#include "Text.h"

using namespace std;

namespace Axodox::Infrastructure
{
  std::string to_lower(std::string_view text)
  {
    string result{ text };
    transform(result.begin(), result.end(), result.begin(), [](char value) { return char(tolower(value)); });
    return result;
  }

  std::wstring to_lower(std::wstring_view text)
  {
    wstring result{ text };
    transform(result.begin(), result.end(), result.begin(), towlower);
    return result;
  }

  std::vector<std::string_view> split(std::string_view text, char delimiter)
  {
    vector<string_view> parts;

    const char* start = text.data();
    for (auto& character : text)
    {
      if (character == delimiter)
      {
        parts.push_back(string_view{ start, &character });
        start = &character + 1;
      }
    }

    if (start < text.data() + text.size())
    {
      parts.push_back(string_view{ start, text.data() + text.size() });
    }

    return parts;
  }
}