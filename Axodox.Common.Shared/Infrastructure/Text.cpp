#include "pch.h"
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
}