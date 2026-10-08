# NamedEnum

`named_enum` is the library's answer to "I want my `enum class` values to round-trip through strings without writing a switch statement". A single macro defines the enum and registers a static serializer that knows the textual name of each value, indexed both by name and by value.

The same machinery is used by:

- The JSON layer to render enum values as strings instead of numbers (see [Json](../Json.md)).
- The free `to_string<T>` / `try_parse<T>` template helpers, which dispatch to the serializer for enums and to `std::format` / `std::from_chars` for arithmetic types.

## Macros

Two macros register an enum and its serializer in one go:

| Macro | Used for |
| --- | --- |
| `named_enum(Type, A, B = 5, C, …)` | Enums — the values are those of the declaration, so `0, 1, 2, …` unless given explicitly. |
| `named_flags(Type, None = 0, A = 1, B = 2, AB = A \| B, …)` | Flags enums — give the bit values explicitly; combinations without a name of their own are written as `A \| B`. |

Both expand to:

```cpp
enum class Type { A, B, C, … };
inline const Axodox::Infrastructure::named_enum_serializer<Type>
  __named_enum_Type{ "A, B, C, …", flags };
```

The serializer parses the declaration the first time its names are needed, so they are also available to other static initializers, and stores both the original name (preserving case) and a lower-cased lookup key for each entry. An entry is `Name` or `Name = value`, where the value is a decimal or `0x` hexadecimal number, a `~` or `-` of one, or a `|` combination of numbers and earlier names. An entry without a value follows the previous one, as in C++. Other initializers, such as shifts or parentheses, are not supported and throw `std::logic_error` when the names are first used.

## What the serializer gives you

`named_enum_serializer<T>` exposes these statics:

```cpp
static std::string to_string(T value);                          // "Foo" or "<numeric>" if unknown
static std::optional<T> try_parse(std::string_view name);    // case-insensitive; also accepts a number
static bool exists();                                           // false until first registration runs
```

`try_parse` accepts either a textual name (case-insensitive) or a numeric string (parsed via `std::from_chars`, which must consume all of it). For `named_flags` it also accepts a `|` combination of these, such as `"Read | Write"`. It returns an empty optional for anything else.

`to_value`, the free `parse<T>` and `invalid_value` are deprecated. They return `T(~0ull)` for invalid text, which cannot be told apart from a flag with all bits set, such as `All = ~0`.

`to_string` returns the name of the value. For `named_flags`, a value without a name of its own is written as its single bit names joined with `" | "`, and falls back to the number when it has bits without a name.

The free `Infrastructure::to_string<T>(value)` and `Infrastructure::try_parse<T>(text)` templates route through the serializer for enums and through `std::format` / `std::from_chars` for integral types, so generic code can write the same call for both:

```cpp
template <typename T>
std::string format_setting(const T& value)
{
  return Axodox::Infrastructure::to_string<T>(value);
}
```

## Code examples

### Defining a sequential enum

```cpp
#include "Include/Axodox.Infrastructure.h"

namespace MyApp
{
  named_enum(LogLevel, Trace, Debug, Info, Warn, Error);
}
```

After this declaration `LogLevel` is a normal `enum class` with the values you'd expect, and the serializer is ready to use:

```cpp
using namespace Axodox::Infrastructure;

auto text = named_enum_serializer<LogLevel>::to_string(LogLevel::Info);   // "Info"
auto back = try_parse<LogLevel>("info");                                 // case-insensitive
auto same = try_parse<LogLevel>("2");                                    // numeric
auto none = try_parse<LogLevel>("verbose");                              // empty optional
```

### Defining a flags enum

Give the bit values explicitly, as the enum itself uses them. Combine the bitwise helpers from [BitwiseOperations](BitwiseOperations.md) for testing and updating bits:

```cpp
named_flags(FilePermissions, None = 0, Read = 1, Write = 2, Execute = 4, Delete = 8);

using namespace Axodox::Infrastructure;

FilePermissions p = bitwise_or(FilePermissions::Read, FilePermissions::Write);
bool canRead      = has_flag(p, FilePermissions::Read);
```

### Logging enum values

Because the logger uses `std::format`, the cleanest way to emit an enum is to format it through the serializer:

```cpp
using namespace Axodox::Infrastructure;

_logger.log(log_severity::information, "Setting log level to {}...",
  named_enum_serializer<LogLevel>::to_string(level));
```

### Round-tripping through JSON

Once an enum is registered as `named_enum`, the JSON layer automatically serialises it as a string and deserialises either form (string or number). No additional code is needed beyond the macro itself — see the [Json](../Json.md) document for the underlying specialization.

```cpp
struct Settings : public Axodox::Json::json_object_base
{
  Axodox::Json::json_property<LogLevel> Level;
  Settings();
};
// → JSON: { "Level": "Info" }
```

## Tips

- **The macro must appear in a place where static initialisation runs** — namespace scope or class scope. Because the serializer is `inline const`, it is safe to put `named_enum(...)` in a header and include it from many translation units.
- **Define each enum once.** Multiple `named_enum(SameType, …)` declarations in different translation units would each try to register the same `T` and trample each other's name lists.
- **`exists()` returns `false` until the static has been initialised**, which can matter during very early `DllMain`-time code paths. Regular application code should not see this.
- **For a comma-separated string of values inside a macro, double parentheses are not needed**: `named_enum(MessageType, A, B, C)` works because the macro is variadic.

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/NamedEnum.h](../../Axodox.Common.Shared/Infrastructure/NamedEnum.h) | The `named_enum` / `named_flags` macros, the `named_enum_serializer<T>` template, and the free `to_string<T>` / `try_parse<T>` helpers for enums and integrals. |
| [Infrastructure/Text.h](../../Axodox.Common.Shared/Infrastructure/Text.h) / [.cpp](../../Axodox.Common.Shared/Infrastructure/Text.cpp) | `to_lower`, `split` and `trim` — used internally to parse the declaration and build the case-insensitive lookup key. See [Text](Text.md). |
| [Json/JsonNumber.h](../../Axodox.Common.Shared/Json/JsonNumber.h) | The JSON serializer specialization that opportunistically uses `named_enum_serializer<T>::exists()` to encode enums as strings. |
