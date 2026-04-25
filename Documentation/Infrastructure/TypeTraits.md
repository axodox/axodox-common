# Traits, Concepts and Type Registry

A small reflection-lite toolkit. Three headers cooperate to detect template instantiations, smart-pointer-like types, and to maintain runtime registries of polymorphic derived classes.

These primitives are mostly internal — the JSON layer, dependency container, and stream API rely on them — but they are useful in any code that needs to specialise behaviour based on what a type *looks like* without imposing a specific base class.

## What is in each header

### `Traits.h`

Type-trait utilities, all in `namespace Axodox::Infrastructure`:

- `is_instantiation_of<Template, T>` / `is_instantiation_of_v<Template, T>` — detects whether `T` is `Template<...>`, e.g. `is_instantiation_of_v<std::vector, T>`.
- `supports_new<T>` — `std::true_type` when `new T()` is well-formed (default-constructible).
- `supports_equals<T>` / `supports_not_equals<T>` — whether `T::operator==` / `T::operator!=` exists.
- `pointed<T>` / `pointed_t<T>` — peels through raw pointers and any type with a nested `element_type` (so `std::shared_ptr<U>`, `std::unique_ptr<U>`, `Infrastructure::value_ptr<U>` all yield `U`). Yields `void` for non-pointer types.

### `Concepts.h`

C++20 concepts that compose the traits with library-specific shapes:

- `supports_to_string<T>` — `T` exposes `value.to_string() -> std::string`.
- `supports_from_string<T>` — `T` exposes `T::from_string(std::string_view) -> std::optional<T>`.
- `supports_to_from_string<T>` — both of the above (used by the JSON string serializer).
- `trivially_copyable<T>` — `std::is_trivially_copyable_v<T>`. Used by stream `read<T>` / `write<T>`.
- `is_pointer_type<T>` — raw pointer, `std::unique_ptr<U>`, or `std::shared_ptr<U>`.
- `is_pointing<T>` — `pointed_t<T>` is not `void`. Captures any pointer-like type, including `value_ptr<U>`.

### `TypeKeySource.h`

`type_key_source<T>` extracts a discriminator from a polymorphic value. The default primary template `static_assert`s, so an unsupported type is a compile error rather than silently returning `0`. The concept-driven specialization picks one of two methods depending on which the type defines:

- `has_lowercase_type<T>` — `value.type()` returns something convertible to `uint32_t`.
- `has_uppercase_type<T>` — `value.Type()` returns something convertible to `uint32_t`.

### `TypeRegistry.h`

`type_registry<TBase>` is a runtime catalogue of derived types keyed by the `type_key_source<TBase>` value:

- Created via the variadic factory `type_registry<TBase>::create<TDerived1, TDerived2, …>()`. Each derived type is default-constructed once at registration time so the registry can read its key.
- `create_unique(uint32_t key)` and `create_shared(uint32_t key)` instantiate a derived type by its key and return a `std::unique_ptr<TBase>` / `std::shared_ptr<TBase>` respectively.
- `get_index(value)` and `get_key(value)` pull the key out of an existing instance.
- The companion concept `has_derived_types<T>` — `static_cast<type_registry<T>&>(T::derived_types)` is well-formed — gates the JSON polymorphism specialization.

## Code examples

### Template-instance detection

`is_instantiation_of_v` is the right tool when you want a single specialisation that fires for *any* `std::vector<T>` (regardless of `T`):

```cpp
template <typename T>
  requires Axodox::Infrastructure::is_instantiation_of<std::vector, T>::value
struct json_serializer<T> { /* … */ };
```

### Detecting equality support

`supports_equals` is used by `value_ptr<T>` to opt into `operator==` only when the pointee defines one:

```cpp
template <typename = std::enable_if_t<
  Axodox::Infrastructure::supports_equals<TValue>::value>>
bool operator==(const value_ptr<TValue>& other) const noexcept;
```

### Concept-gated function templates

The stream layer uses `trivially_copyable` to provide a fast path for POD types:

```cpp
template <Axodox::Infrastructure::trivially_copyable T>
void write(const T& value)
{
  write({ reinterpret_cast<const uint8_t*>(&value), sizeof(T) });
}
```

`supports_to_from_string` does the same job for the JSON string serializer — any type with the conventional `to_string()` / `from_string()` pair becomes JSON-encodable as a string without any further code:

```cpp
template <Axodox::Infrastructure::supports_to_from_string value_t>
struct json_serializer<value_t>
{
  static Infrastructure::value_ptr<json_value> to_json(const value_t& value);
  static bool from_json(const json_value* json, value_t& value);
};
```

### A polymorphic type registry

The pattern is: an abstract base exposes a `Type()` (or `type()`) method returning some discriminator, plus a `static type_registry<Base> derived_types`. Each derived type returns a unique discriminator and is default-constructible. Populate the registry once at namespace scope:

```cpp
struct Shape
{
  virtual int Type() const = 0;
  virtual ~Shape() = default;

  static Axodox::Infrastructure::type_registry<Shape> derived_types;
};

struct Circle : public Shape { int Type() const override { return 1; } };
struct Square : public Shape { int Type() const override { return 2; } };

inline Axodox::Infrastructure::type_registry<Shape>
  Shape::derived_types =
    Axodox::Infrastructure::type_registry<Shape>::create<Circle, Square>();

// Reconstruct by key:
auto shape = Shape::derived_types.create_unique(1);   // -> std::unique_ptr<Shape> wrapping Circle

// Look up the key for an existing instance:
Square s;
auto key = Shape::derived_types.get_key(&s);          // -> 2
```

This is what backs the `"$type"`-discriminated JSON polymorphism described in the [Json](../Json.md) document.

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/Traits.h](../../Axodox.Common.Shared/Infrastructure/Traits.h) | `is_instantiation_of`, `supports_new`, `supports_equals`, `supports_not_equals`, `pointed` / `pointed_t`. |
| [Infrastructure/Concepts.h](../../Axodox.Common.Shared/Infrastructure/Concepts.h) | C++20 concepts: `supports_to_string`, `supports_from_string`, `supports_to_from_string`, `trivially_copyable`, `is_pointer_type`, `is_pointing`. |
| [Infrastructure/TypeKeySource.h](../../Axodox.Common.Shared/Infrastructure/TypeKeySource.h) | `type_key_source<T>`, `has_lowercase_type`, `has_uppercase_type` concepts. |
| [Infrastructure/TypeRegistry.h](../../Axodox.Common.Shared/Infrastructure/TypeRegistry.h) | `type_registry<TBase>` and the `has_derived_types<T>` concept. |
