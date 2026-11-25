#include "common_includes.h"
#include "NamedEnum.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace
{
  uint64_t evaluate_value_operand(const typeless_enum_definition& definition, string_view operand)
  {
    if (operand.starts_with('~')) return ~evaluate_value_operand(definition, trim(operand.substr(1)));
    if (operand.starts_with('-')) return uint64_t(-int64_t(evaluate_value_operand(definition, trim(operand.substr(1)))));

    if (!operand.empty() && isdigit(operand[0]))
    {
      auto base = 10;
      if (operand.size() > 2 && operand[0] == '0' && (operand[1] == 'x' || operand[1] == 'X'))
      {
        operand.remove_prefix(2);
        base = 16;
      }

      uint64_t value;
      auto [end, error] = from_chars(operand.data(), operand.data() + operand.size(), value, base);
      if (error == errc{} && string_view(end, operand.data() + operand.size()).find_first_not_of("uUlL") == string_view::npos) return value;
    }
    else
    {
      for (auto& item : definition.items)
      {
        if (item.name == operand) return item.value;
      }
    }

    throw logic_error(format("Unsupported value '{}' in the declaration of a named enum.", operand));
  }

  uint64_t evaluate_value(const typeless_enum_definition& definition, string_view expression)
  {
    uint64_t result = 0;
    for (auto operand : split(expression, '|'))
    {
      result |= evaluate_value_operand(definition, trim(operand));
    }

    return result;
  }
}

namespace Axodox::Infrastructure
{
  void typeless_enum_definition::add(std::string_view name, uint64_t value)
  {
    items.push_back({ .name = string(name), .key = to_lower(name), .value = value });
  }

  typeless_enum_definition parse_named_enum_definition(std::string_view declaration, bool is_flags)
  {
    typeless_enum_definition result{ .is_flags = is_flags };

    uint64_t next = 0;
    for (auto entry : split(declaration, ','))
    {
      auto separator = entry.find('=');
      auto name = trim(entry.substr(0, separator));
      if (name.empty()) continue;

      auto value = separator == string_view::npos ? next : evaluate_value(result, entry.substr(separator + 1));
      result.add(name, value);

      next = value + 1;
    }

    return result;
  }
}
