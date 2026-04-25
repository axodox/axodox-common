# Coding Conventions

These are the conventions actually used across `axodox-common`, derived from the existing source. They are not aspirational — when a new file is added, it should match the patterns below so the codebase stays consistent. Where two styles coexist, both are flagged and the rule explaining when each applies is given.

## 1. Language and compiler

- **C++20**, conformance mode on, `WarningLevel=Level3`, `SDLCheck=true`. New code may freely use C++20 features (`<format>`, `<span>`, `<ranges>`, concepts, requires-clauses, designated initializers, `[[nodiscard]]`).
- **Precompiled header**: every translation unit either is or transitively includes [common_includes.h](../Axodox.Common.Shared/common_includes.h). The first `#include` in every `.cpp` is `"common_includes.h"`.
- **Encoding**: source is plain ASCII / UTF-8 without BOM; line endings are CRLF (matching the rest of the Visual Studio toolchain).

## 2. File organization

### File-per-type

Each public class, struct or coherent template family lives in its own pair of files: `Foo.h` + `Foo.cpp`. Header-only templates (e.g. [BlockingCollection.h](../Axodox.Common.Shared/Threading/BlockingCollection.h), [ValueBag.h](../Axodox.Common.Shared/Collections/ValueBag.h), [LockedPtr.h](../Axodox.Common.Shared/Threading/LockedPtr.h)) skip the `.cpp`.

### Filenames are PascalCase

Files use **PascalCase** even when the type they contain is `snake_case`:

| File | Primary type |
| --- | --- |
| `Logger.h` | `class logger` |
| `BlockingCollection.h` | `template<typename T> class blocking_collection` |
| `ValueBag.h` | `class value_bag` |
| `JsonObject.h` | `struct json_object` |
| `GraphicsDevice.h` | `class GraphicsDevice` |

So filename casing does **not** follow the casing of the contained identifier — it is a separate convention.

### Folder = namespace = module

Top-level folders under `Axodox.Common.Shared/` correspond 1:1 to namespaces under `Axodox::` (`Collections`, `Graphics`, `Infrastructure`, `Json`, `Storage`, `Threading`, `UI`). The `Graphics` module is further subdivided into thematic subfolders (`Buffers/`, `Devices/`, `Math/`, `Meshes/`, `Shaders/`, `States/`, `Swap Chains/`, `Textures/`); these subfolders do **not** introduce a nested namespace — everything still lives in `Axodox::Graphics`.

> Note: `Swap Chains/` contains a literal space. Keep it. Project files reference the path verbatim.

### Public umbrella headers

Each module ships exactly one umbrella header in `Axodox.Common.Shared/Include/` named `Axodox.<Module>.h`. New modules should follow this pattern. The umbrella simply `#include`s the per-feature headers, with `#ifdef PLATFORM_WINDOWS` (or other) guards around platform-restricted ones. Consumers are documented to include only umbrella headers; per-feature headers are an implementation detail of the umbrella.

## 3. Header structure

Every header begins with:

```cpp
#pragma once
#include "common_includes.h"        // or a sibling header that pulls it in
```

Header guards via `#pragma once` only. Never use include-once macros.

When a header transitively needs `common_includes.h` already (via `#include "Stream.h"` etc.), the explicit `#include "common_includes.h"` is omitted — see [FileStream.h](../Axodox.Common.Shared/Storage/FileStream.h).

Platform-specific headers wrap their entire body in `#ifdef PLATFORM_WINDOWS` (and, where relevant, `WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)` for desktop-only features). See [Win32.h](../Axodox.Common.Shared/Infrastructure/Win32.h) and [Window.h](../Axodox.Common.Shared/UI/Window.h) for the canonical patterns.

DLL-exported types are tagged with `AXODOX_COMMON_API` immediately before the type name:

```cpp
class AXODOX_COMMON_API logger { … };
struct AXODOX_COMMON_API AdapterInfo { … };
AXODOX_COMMON_API std::vector<uint8_t> read_file(const std::filesystem::path& path);
```

Free functions intended for consumers also carry `AXODOX_COMMON_API`. Header-only templates do not.

## 4. Source files

`.cpp` files start with:

```cpp
#include "common_includes.h"
#include "TheirOwnHeader.h"
#include "OtherDependencies.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace std;
using namespace std::chrono;

namespace
{
  // file-private helpers go here, in an anonymous namespace
}

namespace Axodox::TheModule
{
  // implementations
}
```

`using namespace` directives at file scope inside `.cpp` files are normal and expected (`std`, `std::chrono`, `winrt`, and the relevant `Axodox::*` namespaces). Do not use them in headers.

File-private helpers — types, free functions, statics, lambdas, RAII initializer/finalizer pairs — go inside an unnamed `namespace { … }` block at the top of the `.cpp`, *not* into the `Axodox::*` namespace. See [Logger.cpp](../Axodox.Common.Shared/Infrastructure/Logger.cpp) for the canonical pattern, including the use of `lifetime_executor<initialize_logger, shutdown_logger>` to register module init/shutdown callbacks.

## 5. Naming

### The two casing styles

The codebase uses **two parallel naming styles**, split by module:

1. **`snake_case`** — used by `Infrastructure`, `Threading`, `Collections`, `Storage`, `Json`, and `UI` (free functions only). Type names, member functions, free functions, locals, parameters, enum-class members are all `snake_case`. Template parameters use `TPascalCase` (e.g. `TArgs`, `TInstance`) or `value_t` / `converter_t` (in Json, where the alias style is preferred).
2. **`PascalCase`** — used by the `Graphics` module and the `UI::Window` class. Type names, methods, public fields, and enum-class members are all `PascalCase`. Template parameters are `PascalCase` (`ItemType`, `CapacityOrImmutableData`).

When adding a file, **match the casing style of the surrounding module**. A new file in `Graphics/` is PascalCase; a new file in `Threading/` is snake_case. Do not mix the two styles within a single type.

### Member fields

Private/protected fields are prefixed with a leading underscore: `_value`, `_handlers`, `_mutex`, `_device`, `_capacity`, `_size`. This rule applies to **both** casing styles.

```cpp
class logger { … private: std::string_view _channel; };
class GraphicsBuffer { … protected: winrt::com_ptr<ID3D11Buffer> _buffer; uint32_t _capacity; };
```

Public fields on plain-data structs (e.g. `AdapterInfo`, `Point`, `async_operation_info`) are not prefixed.

### Enums

Always `enum class` — never bare `enum`. The casing of members matches the module's casing style:

```cpp
// Infrastructure / Threading / Storage / Json — snake_case
enum class log_severity { debug, information, warning, error, fatal };
enum class file_mode : uint8_t { none = 0, read = 1, write = 2, read_write = read | write };

// Graphics — PascalCase, often a flags enum with explicit power-of-two values
enum class GraphicsDeviceFlags : uint32_t {
  None = 0,
  UseDebugSdkLayers = 1,
  UseDebuggableDevice = 2,
  UseWarpDevice = 4,
  UseDirect3D11On12Device = 8
};
```

Specify an explicit underlying type (`: uint8_t`, `: uint32_t`) when the enum is a flags enum or has bit-layout meaning.

For string-reflectable enums, use the `named_enum(...)` / `named_flags(...)` macros from [NamedEnum.h](../Axodox.Common.Shared/Infrastructure/NamedEnum.h) rather than rolling a manual `to_string`/`from_string` pair.

### Common name suffixes

- `_t` — type aliases used as parameter types or short typedefs (`token_t`, `handler_t`, `deleter_t`, `arg_t`, `value_t`).
- `_base` — abstract base classes (`json_value`'s collaborators use `json_object_base`, `dependency_registration_base`, `event_handler_collection_base`).
- `_info` — POD descriptor structs (`async_operation_info`, `AdapterInfo`).
- `2D` — 2D graphics resources (`Texture2D`, `RenderTarget2D`, `DepthStencil2D`).

### Smart pointer / handle types

The library defines several pointer abstractions; default to standard library types and reach for these only when their semantics are needed:

- `Infrastructure::value_ptr<T>` — owning pointer with deep copy. Used pervasively by the Json DOM.
- `Infrastructure::dependency_ptr<T>` — usually `std::shared_ptr<T>`, returned by the DI container.
- `winrt::com_ptr<T>` — DirectX / WinRT COM ownership.
- `winrt::handle_type<…>` — typed Win32 HANDLE wrappers.

## 6. Formatting

- **Indentation**: 2 spaces, no tabs.
- **Braces**: Allman style (open brace on its own line) for everything — namespaces, classes, functions, control flow, lambdas (when multi-line), and even single-line `if` bodies are typically wrapped in braces. Single-statement guards like `if (!_value) return;` may keep the body on the same line and omit braces.
- **Pointer/reference**: `T* p`, `T& r`, `T&& r` — the punctuation hugs the type, not the identifier.
- **One declaration per line**, except for trivial homogeneous data members (e.g. `int32_t X, Y;` in [Point.h](../Axodox.Common.Shared/Graphics/Math/Point.h)).
- **Member initializer lists**: each initializer on its own line, comma-leading the next, opening brace on the line below the last initializer:

  ```cpp
  dependency_registration(dependency_container* owner, dependency_lifetime lifetime) :
    _owner(owner),
    _lifetime(lifetime)
  {
    _factory = dependency_pointer_type<T>::get_factory(owner);
  }
  ```

- **Designated initializers** are preferred when constructing aggregates (`item_t{ .value = …, .deleter = … }`, `log_entry{ .time = …, .severity = …, .channel = …, .text = … }`).
- **Trailing returns and `auto`**: ordinary functions use leading return types; lambdas usually omit the return type unless needed.
- **`const` placement**: east-const is not used. Write `const T&`, `const std::string&`, `const winrt::com_ptr<…>&`.
- **`noexcept`**: applied liberally to move constructors/assignments, destructors, and trivially-non-throwing accessors. Match the surrounding style — if a class consistently marks `noexcept`, new methods should too.
- **`[[nodiscard]]`**: applied to RAII tokens whose return value must not be ignored, e.g. `event_subscription`, `lifetime_token`.

## 7. API design patterns

### Three-layer access for resource wrappers

DirectX wrappers expose three accessors:

```cpp
const winrt::com_ptr<ID3D11DeviceT>& operator*() const;  // shared ownership
ID3D11DeviceT* operator->() const;                       // member access
ID3D11DeviceT* get() const;                              // raw pointer
```

Follow this trio when wrapping any COM/HANDLE resource.

### Property-style getters and setters

Pairs of overloads share a name, distinguished by signature:

```cpp
static log_severity severity();
static void severity(log_severity value);

async_operation_info state() const;
```

Avoid `get_` / `set_` prefixes for these. Setters take their value by value or by `std::string_view` rather than by `const std::string&`.

### Try/throw pairs

Operations that can fail come in two flavors when both make sense:

```cpp
std::vector<uint8_t> read_file(const std::filesystem::path& path);     // throws on error
std::vector<uint8_t> try_read_file(const std::filesystem::path& path); // returns empty / optional
std::optional<std::string> try_read_text(const std::filesystem::path& path);
```

The `try_` prefix means "may fail without throwing"; the return type tells the caller how (empty container, `std::optional`, or `bool` plus out-parameter).

### Out parameters via reference

Where a `try_` function needs to return both a status and a value, the value goes through an out-reference:

```cpp
bool try_get_value(const char* key, json_value*& json) const;
bool try_get(T& item, event_timeout timeout = {});
```

### Events

Public events are exposed as `Infrastructure::event_publisher<Args…>` member objects. The owning class holds a private `Infrastructure::event_owner _events` and raises through it:

```cpp
class async_operation_source {
  Infrastructure::event_owner _events;
public:
  Infrastructure::event_publisher<const async_operation_info&> state_changed;
  …
};
```

Subscribers take ownership of an `event_subscription` (which is `[[nodiscard]]`) and unsubscribe automatically on destruction. Use `subscribe(no_revoke, …)` only when the event source is owned by the subscribing class.

### Polymorphic deletion / dispatch

`json_value` and friends use a manual virtual hierarchy with `to_string` / `from_string` overrides. New polymorphic value types should follow the same shape: `struct foo : public json_value_container<…>`.

For runtime polymorphism with type discovery (e.g. discriminated unions in JSON), use `Infrastructure::TypeRegistry` + `derived_types`. The Json layer integrates this with `$type`-discriminated objects out of the box (see [JsonSerializer.h](../Axodox.Common.Shared/Json/JsonSerializer.h)).

## 8. Concurrency style

- Lock with `std::lock_guard`, `std::unique_lock`, or `std::shared_lock` over `std::mutex` / `std::shared_mutex`. RAII locks only — no manual `lock()` / `unlock()` calls in normal code paths.
- Long-lived background workers are owned by RAII helpers in an anonymous namespace, started/stopped by `Infrastructure::lifetime_executor<Init, Shutdown>` (see [Logger.cpp](../Axodox.Common.Shared/Infrastructure/Logger.cpp)).
- `Threading::manual_reset_event` + `Threading::blocking_collection<T>` are the preferred building blocks for producer/consumer patterns; do not roll new condvar code.

## 9. Comments and documentation

- Keep comments rare. The codebase prefers self-describing identifiers. Comments are reserved for non-obvious *why*: see the inline `//Do not move out of header - it won't work that way` note above `lib_folder()` in [FileIO.h](../Axodox.Common.Shared/Storage/FileIO.h).
- No file-level banners, license headers, or `@author` blocks. The repository LICENSE is the source of truth.
- No Doxygen / Javadoc comments. Public API discoverability comes from the umbrella headers and the typed surface itself.

## 10. Practical checklist for adding new code

1. Decide the module (and therefore the casing style and folder).
2. Create `Foo.h` + `Foo.cpp` (omit the `.cpp` only if the type is fully header-only).
3. In the header: `#pragma once`, include the right transitive header, declare in `namespace Axodox::<Module> { … }`, tag exported types with `AXODOX_COMMON_API`, prefix private members with `_`.
4. In the `.cpp`: `#include "common_includes.h"` first, then the matching header, then dependencies; pull namespaces in with `using namespace …`; put file-private helpers in an anonymous namespace; implement inside `namespace Axodox::<Module>`.
5. Add both files to [Axodox.Common.Shared.vcxitems](../Axodox.Common.Shared/Axodox.Common.Shared.vcxitems) under the matching `<ItemGroup>` (`ClInclude` / `ClCompile`).
6. Add a `#include` line to the matching umbrella header in `Include/Axodox.<Module>.h`, wrapped in the right `#ifdef` if it's platform-restricted.
7. If the new code requires a Windows or DirectX dependency, gate the header body with `#ifdef PLATFORM_WINDOWS` (and `#if defined(USE_DIRECTX) && defined(PLATFORM_WINDOWS)` for D3D-specific types) so the umbrella header stays consistent with [common_includes.h](../Axodox.Common.Shared/common_includes.h).
