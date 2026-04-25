# Uuid

`uuid` is a 16-byte value type for working with GUIDs in their canonical Microsoft mixed-endian textual form (`xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`). It does not generate UUIDs by itself — for that, use [`make_guid()`](Win32.md) on Windows.

## Anatomy

```cpp
struct uuid
{
  std::array<uint8_t, 16> bytes;

  uuid();                                              // zero-initialized
  uuid(std::string_view text);                         // throws std::invalid_argument on bad input
  operator std::string() const;
  std::string to_string() const;
  static std::optional<uuid> from_string(std::string_view text);
};
```

The byte order matches Windows' `GUID` layout — the first three groups are little-endian, the last two are big-endian. Reading or writing GUIDs through this type therefore round-trips exactly to and from `winrt::guid`, registry strings, and SDK functions.

## Code examples

### Constructing

```cpp
#include "Include/Axodox.Infrastructure.h"

using namespace Axodox::Infrastructure;

uuid empty;                                            // all 16 bytes zero
uuid known{ "01234567-89ab-cdef-0123-456789abcdef" };  // throws on malformed input

auto parsed = uuid::from_string("01234567-89ab-cdef-0123-456789abcdef");
if (parsed) UseIt(*parsed);                            // non-throwing variant
```

### Stringifying

The implicit `std::string` conversion makes UUIDs printable without ceremony, and the `to_string()` form composes nicely with `std::format`:

```cpp
uuid id = ReceiveFromWire();
std::string text = id;                                 // implicit -> "01234567-89ab-..."

_logger.log(log_severity::information, "Session id: {}", id.to_string());
```

### Generating new UUIDs

`uuid` itself is just a value type. To produce a fresh GUID, use the platform helper from [Win32](Win32.md) (Windows-only) and copy its bytes:

```cpp
#ifdef PLATFORM_WINDOWS
auto guid = Axodox::Infrastructure::make_guid();       // winrt::guid
uuid id;
std::memcpy(id.bytes.data(), &guid, sizeof(guid));     // sizes match
#endif
```

A typical "use a session id from the command line, or generate one if missing" pattern looks like this:

```cpp
auto sessionId = arguments.SessionId
  ? *arguments.SessionId                               // user-supplied via CLI
  : uuid_from_winrt(make_guid());                      // freshly generated otherwise
```

### Round-tripping through JSON

`uuid` satisfies the `supports_to_from_string` concept (because it has both `to_string()` and `static from_string(std::string_view)` returning `std::optional<uuid>`), so the JSON layer automatically serialises it as a string without any extra registration:

```cpp
struct Session : public Axodox::Json::json_object_base
{
  Axodox::Json::json_property<Axodox::Infrastructure::uuid> Id;
  Session();
};
// → JSON: { "Id": "01234567-89ab-..." }
```

This is what makes `Uuid` a first-class wire-format type in messages built on top of the JSON DOM.

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/Uuid.h](../../Axodox.Common.Shared/Infrastructure/Uuid.h) | The `uuid` value type. |
| [Infrastructure/Uuid.cpp](../../Axodox.Common.Shared/Infrastructure/Uuid.cpp) | Mixed-endian text parsing / formatting. |
| [Infrastructure/Win32.h](../../Axodox.Common.Shared/Infrastructure/Win32.h) | `make_guid()` — generate a fresh `winrt::guid` on Windows. See [Win32](Win32.md). |
| [Infrastructure/Concepts.h](../../Axodox.Common.Shared/Infrastructure/Concepts.h) | `supports_to_from_string` — the concept that lets `uuid` plug into the JSON layer automatically. |
