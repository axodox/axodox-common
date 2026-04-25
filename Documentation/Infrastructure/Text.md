# Text

A pair of small free-function string utilities used across the library and consumer code. Nothing fancy — just the operations the standard library doesn't quite hand you.

## API

```cpp
namespace Axodox::Infrastructure
{
  std::string  to_lower(std::string_view  text);
  std::wstring to_lower(std::wstring_view text);

  std::vector<std::string_view> split(std::string_view text, char delimiter);
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

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/Text.h](../../Axodox.Common.Shared/Infrastructure/Text.h) | `to_lower` (string + wstring) and `split(text, delimiter)` declarations. |
| [Infrastructure/Text.cpp](../../Axodox.Common.Shared/Infrastructure/Text.cpp) | Implementations using `std::transform` and a single-pass scan. |
