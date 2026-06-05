# Text

A handful of small free-function string utilities used across the library and consumer code. Nothing fancy — just the operations the standard library doesn't quite hand you.

## API

```cpp
namespace Axodox::Infrastructure
{
  std::string  to_lower(std::string_view  text);
  std::wstring to_lower(std::wstring_view text);

  std::vector<std::string_view> split(std::string_view text, char delimiter);

  std::string encode_base64(std::span<const uint8_t> data);
  bool try_decode_base64(std::string_view text, std::vector<uint8_t>& data);
}
```

### `to_lower`

Returns a copy of `text` with every character mapped through `tolower` / `towlower`. ASCII-safe; for non-ASCII text, the result depends on the current C locale, like every other `std::tolower` user.

```cpp
auto canonical = Axodox::Infrastructure::to_lower("Hello-World");   // "hello-world"
auto wide      = Axodox::Infrastructure::to_lower(L"ÁRVÍZTŰRŐ");    // locale-dependent
```

This is what `named_enum_serializer<T>` uses internally to build its case-insensitive lookup keys (see [NamedEnum](NamedEnum.md)).

### `split`

Returns a vector of `std::string_view` slices that point into the original buffer. The original `text` must outlive the returned views — no ownership is transferred.

```cpp
auto path  = std::string_view{ "alpha/beta/gamma" };
auto parts = Axodox::Infrastructure::split(path, '/');               // ["alpha", "beta", "gamma"]
```

Behaviour notes:

- Consecutive delimiters produce empty slices: `split("a,,b", ',')` → `["a", "", "b"]`.
- Trailing delimiters do not produce an empty slice at the end: `split("a,b,", ',')` → `["a", "b"]`.
- An empty input string returns an empty vector.

Pair it with `to_lower` for a quick "case-insensitive contains" check:

```cpp
auto haystack = Axodox::Infrastructure::to_lower(input);
for (auto needle : Axodox::Infrastructure::split(searchTerms, ' '))
{
  if (haystack.contains(needle)) return true;
}
```

### `encode_base64` / `try_decode_base64`

Standard (RFC 4648) base64 between a binary buffer and its textual form.

```cpp
namespace ax = Axodox::Infrastructure;

std::vector<uint8_t> bytes = { 'f', 'o', 'o', 'b', 'a', 'r' };
auto text = ax::encode_base64(bytes);              // "Zm9vYmFy"

std::vector<uint8_t> decoded;
if (ax::try_decode_base64(text, decoded))          // round-trips back to the original bytes
{
  // decoded == bytes
}
```

- `encode_base64` takes a `std::span<const uint8_t>`, so any contiguous byte buffer (`std::vector`, `std::array`, C array, …) works without a copy. The output is padded with `=` as needed.
- `try_decode_base64` returns `false` and leaves `data` untouched on malformed input. It is strict: it rejects characters outside the base64 alphabet, any data following the padding, and non-canonical trailing bits (a final group whose unused low bits aren't zero, e.g. `"Zh=="`).
- The two are exact inverses for any byte sequence, including the empty buffer (`""`).

This is the codec behind [`json_base64_converter`](../Json.md#custom-per-property-converters), which travels a `std::vector<uint8_t>` JSON property as a base64 string rather than an array of numbers.

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/Text.h](../../Axodox.Common.Shared/Infrastructure/Text.h) | `to_lower` (string + wstring), `split(text, delimiter)`, and base64 `encode_base64` / `try_decode_base64` declarations. |
| [Infrastructure/Text.cpp](../../Axodox.Common.Shared/Infrastructure/Text.cpp) | Implementations using `std::transform`, a single-pass scan, and a table-driven base64 codec. |
