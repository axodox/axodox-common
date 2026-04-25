# Win32 and WinRT bridges

A handful of Windows-only helpers that paper over the gaps between the standard library, Win32, and WinRT projection types. Each one is small enough to fit in a single declaration; the doc below collects them so that consumer code can find what it needs without grepping the headers.

All of these live in `namespace Axodox::Infrastructure` and are gated by `#ifdef PLATFORM_WINDOWS`.

## `Win32.h`

### `win32_handle_traits<T>` and `window_handle`

`winrt::handle_type<TraitsT>` is a type-safe RAII wrapper around a Win32 `HANDLE`-shaped value. The library ships one set of traits — `win32_handle_traits<T>` — that calls `CloseHandle` on destruction and uses `nullptr` as the invalid sentinel:

```cpp
template <typename T>
struct win32_handle_traits
{
  using type = T;
  static void close(type value) noexcept            { CloseHandle(value); }
  static constexpr type invalid() noexcept          { return nullptr; }
};

using window_handle = winrt::handle_type<win32_handle_traits<HWND>>;
```

`window_handle` is a typedef for the most common case — owning an `HWND`. Use it (or build your own typedef from the same traits) when storing handles as members:

```cpp
class MyWindow
{
  Axodox::Infrastructure::window_handle _hwnd;     // closed automatically on destruction
};
```

For handles whose cleanup is *not* `CloseHandle` (for example, file mapping objects), define your own traits modelled on this template.

### `to_wstring(std::string_view)`

Converts a UTF-8 byte sequence into a `std::wstring` via `MultiByteToWideChar(CP_UTF8, …)`. This is the canonical way the library bridges its UTF-8 internals into the Win32 / WinRT wide-character APIs:

```cpp
auto title = Axodox::Infrastructure::to_wstring(utf8Title);
SetWindowText(hwnd, title.c_str());
```

Returns an empty string on failure (zero-sized input or invalid UTF-8). For the reverse direction, use `winrt::to_string(std::wstring_view)` from the WinRT projection.

### `has_package_identity()`

Returns `true` if the current process is running inside an MSIX / UWP package, `false` otherwise. Implemented via `GetCurrentPackageFullName`. The logger uses it to decide whether to spin up a UWP `FileLoggingSession`:

```cpp
if (Axodox::Infrastructure::has_package_identity())
{
  EnableSandboxedFeatures();
}
```

### `make_guid()`

Generates a fresh `winrt::guid` via `CoCreateGuid`. Throws on failure (uses `winrt::check_hresult`). Combine with [`uuid`](Uuid.md) when you need a textual representation:

```cpp
auto guid = Axodox::Infrastructure::make_guid();   // winrt::guid

Axodox::Infrastructure::uuid id;
std::memcpy(id.bytes.data(), &guid, sizeof(guid));
auto text = id.to_string();                        // "01234567-89ab-..."
```

## `Environment.h`

### `get_environment_variable(std::string_view name)`

Reads a process environment variable, expands embedded `%VARS%` references via `ExpandEnvironmentStringsA`, and returns the result as a UTF-8 `std::string`. Returns an empty string if the variable is missing.

```cpp
using namespace Axodox::Infrastructure;

if (get_environment_variable("MY_APP_CONTAINER").empty())
{
  RunHostInitialization();
}

auto endpoint = get_environment_variable("MY_APP_GATEWAY_URI");
if (!endpoint.empty()) UseCustomEndpoint(endpoint);
```

The lookup is bounded by the Win32 `_MAX_ENV` constant (32 KiB) — large enough for any practical environment variable but not unbounded.

## `WinRtDependencies.h`

This header specialises `dependency_pointer_type<T>` for any WinRT projection type (anything where `winrt::impl::has_category_v<T>` is true). The specialisation:

- Holds the value directly instead of wrapping it in `std::shared_ptr<T>` — WinRT projection types are already reference-counted on their own.
- Constructs default-state values with `T{ nullptr }` so a missing dependency resolves to a `null` projection.
- Routes through the WinRT projection's own copy/cast semantics for `cast<U>`.

Including the header is enough; once it is in scope, `dependency_container::resolve<TWinRtType>()` returns a plain `TWinRtType` value:

```cpp
#include "Infrastructure/WinRtDependencies.h"

using namespace Axodox::Infrastructure;

dependency_container container;
container.add<winrt::Windows::Storage::ApplicationDataContainer>(
  winrt::Windows::Storage::ApplicationData::Current().LocalSettings());

auto settings = container.resolve<winrt::Windows::Storage::ApplicationDataContainer>();
```

For the full container API, see [DependencyContainer](DependencyContainer.md).

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/Win32.h](../../Axodox.Common.Shared/Infrastructure/Win32.h) / [.cpp](../../Axodox.Common.Shared/Infrastructure/Win32.cpp) | `win32_handle_traits<T>`, `window_handle`, `to_wstring(std::string_view)`, `has_package_identity()`, `make_guid()`. |
| [Infrastructure/Environment.h](../../Axodox.Common.Shared/Infrastructure/Environment.h) / [.cpp](../../Axodox.Common.Shared/Infrastructure/Environment.cpp) | `get_environment_variable(name)` returning the expanded value as UTF-8. |
| [Infrastructure/WinRtDependencies.h](../../Axodox.Common.Shared/Infrastructure/WinRtDependencies.h) | `dependency_pointer_type` specialization for WinRT projection types. |
