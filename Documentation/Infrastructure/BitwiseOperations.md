# Bitwise and Memory Utilities

Two tightly related sets of helpers in one header, plus the `half` half-precision float type:

- **Typed flag operators** that work on `enum class` values without requiring per-enum `operator|` / `operator&` overloads.
- **Memory utilities** for trivially-copyable types — zero-fill, byte-level equality, default-value detection, and reinterpretation as `std::span<const uint8_t>`.
- **`half`** — a 16-bit IEEE 754 floating-point wrapper that implicitly converts to and from `float`.

These are header-only templates, so they evaporate after compilation. They are widely used by the Graphics module (flag enums for D3D feature flags, byte-level buffer comparisons, `to_span` for shader uploads) and by the JSON / Storage layers (trivially-copyable detection paths).

## Flag operators on `enum class`

`enum class` is type-safe but does not inherit operators from its underlying integral type. Without help, `MyFlags::A | MyFlags::B` is a compile error. The functions in `BitwiseOperations.h` work by converting through `std::underlying_type_t<T>`:

```cpp
template <typename T> constexpr T   bitwise_or    (T a, T b);
template <typename T> constexpr T   bitwise_and   (T a, T b);
template <typename T> constexpr T   bitwise_negate(T a);

template <typename T> constexpr bool has_flag    (T a, T b);   // (a & b) == b
template <typename T> constexpr bool has_any_flag(T a, T b);   // (a & b) != 0

template <typename T> void add_flag(T& a, T b, bool value = true);  // ORs in if value
template <typename T> void set_flag(T& a, T b, bool value);         // sets or clears
```

### Examples

Pair these with `named_flags(...)` for an `enum class` you can both string-format and bitwise-combine:

```cpp
named_flags(FilePermissions, Read, Write, Execute, Delete);

using namespace Axodox::Infrastructure;

FilePermissions p{};
add_flag(p, FilePermissions::Read);
add_flag(p, FilePermissions::Write);

if (has_flag(p, FilePermissions::Read)) OpenForReading();

set_flag(p, FilePermissions::Execute, isExecutable);  // toggles based on bool
```

The `file_mode` enum from the Storage module uses `has_any_flag` internally to validate which operations a `file_stream` allows.

## Memory utilities

### Zero-fill

```cpp
template <typename T> void zero_memory(T& value);
```

`memset`s the value to all zeros. `static_assert`s that `T` is trivially copyable, so it cannot be misused on a type with a vtable or non-trivial members.

```cpp
D3D11_TEXTURE2D_DESC desc;
zero_memory(desc);
desc.Width = 512;
```

### Byte-level equality

```cpp
template <typename T> bool are_equal(const T& a, const T& b);
template <typename T> bool are_equal(std::span<const T> a, std::span<const T> b);
template <typename U, typename V> bool are_equal(const U& a, const V& b);   // sizeof(U) == sizeof(V)

#ifdef WINRT_Windows_Foundation_H
bool are_equal(const winrt::Windows::Foundation::IInspectable& a,
               const winrt::Windows::Foundation::IInspectable& b);
#endif
```

The single-value form picks `memcmp` for trivially-copyable types and `operator==` otherwise. The cross-type form requires the two operands to have the same size — useful for comparing `LUID` against a manually-built `{ LowPart, HighPart }` pair, or for memcmp-comparing two POD records that the compiler does not consider the same type.

```cpp
if (Axodox::Infrastructure::are_equal(adapterDesc.AdapterLuid, expectedLuid))
{
  ChooseThisAdapter();
}
```

`is_default(value)` is shorthand for "did anyone set this?":

```cpp
if (Axodox::Infrastructure::is_default(handle)) return;
```

### Reinterpreting as bytes

Several patterns in the library serialise trivially-copyable values to byte ranges. The `to_span` / `to_vector` family centralises this:

```cpp
template <typename T> std::span<const uint8_t>   to_span(const T& value);
template <typename T> std::span<const uint8_t>   to_span(std::span<const T> span);
template <typename T> std::span<const uint8_t>   to_span(std::initializer_list<T> list);

template <typename T> std::vector<uint8_t>       to_vector(std::span<const T> span);
template <typename T> std::vector<uint8_t>       to_vector(std::initializer_list<T> list);
```

```cpp
struct Vertex { float x, y, z; };
std::vector<Vertex> mesh = { /* … */ };

device->UploadVertexBuffer(Axodox::Infrastructure::to_span<Vertex>(mesh));

// initializer-list overload — handy for inline constants
auto bytes = Axodox::Infrastructure::to_vector<uint16_t>({ 0xCAFE, 0xBABE });
```

The `initializer_list` overloads compute the span as `[list.begin(), list.end())`, so they only work for genuinely contiguous sequences (which `std::initializer_list` always is).

## `half`

`half` is a Windows-only thin wrapper around `uint16_t` representing an IEEE 754 binary16 value. It exposes only two operations: assignment from `float`, and implicit conversion to `float`. The conversion uses DirectXMath's packed-vector path internally, so it is bit-exact with HLSL's `half`.

```cpp
#ifdef PLATFORM_WINDOWS
Axodox::Infrastructure::half h;
h = 1.5f;                                      // store as binary16
float f = h;                                   // convert back to single precision
#endif
```

Use it when interacting with shaders, FP16 textures, or wire formats that store half-precision values; everywhere else, prefer `float`.

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/BitwiseOperations.h](../../Axodox.Common.Shared/Infrastructure/BitwiseOperations.h) | Typed flag operators (`bitwise_or` / `bitwise_and` / `bitwise_negate`, `has_flag` / `has_any_flag`, `add_flag` / `set_flag`), `zero_memory`, `are_equal` family (with optional WinRT `IInspectable` overload), `is_default`, `to_span` / `to_vector`. |
| [Infrastructure/BitwiseOperations.cpp](../../Axodox.Common.Shared/Infrastructure/BitwiseOperations.cpp) | Out-of-line definitions for the WinRT overloads. |
| [Infrastructure/Half.h](../../Axodox.Common.Shared/Infrastructure/Half.h) / [.cpp](../../Axodox.Common.Shared/Infrastructure/Half.cpp) | Windows-only `half` (IEEE binary16) with implicit conversions to and from `float`. |
| [Infrastructure/NamedEnum.h](../../Axodox.Common.Shared/Infrastructure/NamedEnum.h) | `named_flags(...)` macro that pairs naturally with these operators. See [NamedEnum](NamedEnum.md). |
