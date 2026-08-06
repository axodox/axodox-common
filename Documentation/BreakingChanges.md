# Breaking Changes

Changes which require consumers to update their code, newest first. Each entry states what
broke, why, and the mechanical fix.

## Unreleased

### JSON: converters must implement `is_default`

**What changed.** Every converter — each `json_serializer<T>` specialization and every custom
per-property converter — must now provide a third static method alongside `to_json` and
`from_json`:

```cpp
static bool is_default(const T& value);
```

A new concept enforces it, and property descriptors are constrained against it:

```cpp
template <typename converter_t, typename value_t>
concept json_converter = requires (const value_t& value, const json_value* json, value_t& target)
{
  { converter_t::to_json(value) }         -> std::convertible_to<Infrastructure::value_ptr<json_value>>;
  { converter_t::from_json(json, target) } -> std::same_as<bool>;
  { converter_t::is_default(value) }       -> std::same_as<bool>;
};
```

**Why.** Properties declared `is_required = false` are omitted from the output while they hold
their default value. Deciding *what counts as default* requires knowing the wire format, which
only the converter knows. The previous implementation guessed from the C++ value using a
compile-time chain of `if constexpr` branches, and fell back to serializing the value and
inspecting the resulting node when it could not decide. That guessing was both slower and wrong
in cases the converter could have answered exactly.

**How to fix.** Add the method to each custom converter. The answer should describe the
*serialized* form, not the C++ value:

```cpp
struct my_converter
{
  static value_ptr<json_value> to_json(const T& value);
  static bool from_json(const json_value* json, T& value);

  //A buffer that encodes to an empty string is default.
  static bool is_default(const T& value) { return value.empty(); }
};
```

A converter that omits it fails at the property definition with a concept error naming the
missing method, rather than deep inside a template instantiation.

**Not affected.** Code that only *uses* the built-in serializers — `stringify_json`,
`try_parse_json`, `describe_json_object` with plain fields — needs no changes. All built-in
`json_serializer<T>` specializations already implement the method.

### JSON: `json_value::is_default()` is a new pure virtual

**What changed.** `json_value` gained `virtual bool is_default() const = 0;`. Every type deriving
from `json_value` must implement it.

**Why.** The default test used to be a free function switching over `json_type` with a
`static_cast` per case. Making it virtual puts each type's answer on the type itself and removes
the possibility of a new value type silently defaulting to "not default".

**How to fix.** Implement the method on any custom `json_value` subclass. Types deriving from
`json_value_container<T, json_type>` are *not* automatically covered — each of the built-in
types declares its own override. Follow the same shape:

```cpp
struct my_value : public json_value_container<my_type, json_type::object>
{
  virtual bool is_default() const override;   //declare in the header
};
```

The free function `json_value_is_default(const json_value*)` still exists as a null-safe wrapper
(`!value || value->is_default()`), so call sites holding a possibly empty pointer are unchanged.
It is now `inline` rather than exported, which matters only if you took its address.

### JSON: `is_required = false` no longer marks a property required in the schema

**What changed.** A property declared `{.is_required = false}` is no longer listed in the
generated JSON Schema's `required` array.

**Why.** A bug: the schema generator tested `if (property.is_required())` on a
`std::optional<bool>`, which asks *is the optional engaged*, not *is it true*. Both `true` and
`false` are engaged, so an explicit `false` was treated as required. It now tests `== true`.

**How to fix.** Nothing to change in your code, but **check any recorded or asserted schema
output** — properties marked `is_required = false` will disappear from `required`. If a test
pins the exact schema JSON, update the expectation.

The three states are now distinct and behave as follows:

| `is_required` | Value is default | Value is not default | Listed in schema `required` |
| --- | --- | --- | --- |
| `true` | serialize | serialize | yes |
| unset (default) | serialize | serialize | no |
| `false` | **omit** | serialize | no |

### JSON: an engaged `std::optional` is never default

**What changed.** For a property with `is_required = false`, an engaged `std::optional<T>` is
always serialized — even when the value it holds is itself a default (`optional<string>{""}`,
`optional<int>{0}`). Only a disengaged optional is omitted.

**Why.** An optional adds `null` to the value's range, so being engaged is itself information.
Emitting nothing and emitting `""` are different statements about the value.

**How to fix.** Nothing to change if this is the behaviour you wanted. If you relied on an
engaged-but-default optional being omitted, use a plain `T` instead of `std::optional<T>` — a
plain field holding the same value *is* omitted.

### Networking: `service_address` has a single `address` member

**What changed.** The struct raised by `service_locator::service_found` went from three members
to two:

```cpp
//before
struct service_address
{
  std::string id;
  socket_address_variant resolvedAddress;
  socket_address_variant senderAddress;
};

//after
struct service_address
{
  std::string id;
  socket_address_variant address;
};
```

Both remaining members are `snake_case`, matching the module's convention.

**Why.** `address` is the address to connect to: the announced address, with a wildcard host
(`0.0.0.0` / `[::]`) replaced by the host the announcement actually arrived from. The separate
sender address exposed a transport detail that the resolved address already accounts for.

**How to fix.** Rename `resolvedAddress` to `address`. For uses of `senderAddress`, see below —
the fix depends on why you were reading it.

**If you used `senderAddress` to check a response came from the local machine**, test the
resolved address instead:

```cpp
if (!is_loopback(info.address)) return;   //reject responders we cannot reach locally
```

This holds however the service was announced:

- Announced on the wildcard host — the resolver substitutes the host the response came from, so
  a local responder resolves to `127.0.0.1` and a remote one to its LAN address.
- Announced on loopback — the resolver passes it through, so `address` stays `127.0.0.1`. A
  remote responder announcing loopback is advertising an address only valid on its own machine,
  so it is unusable to us regardless; treating it as local-only is the correct outcome.

Either way the check answers the question that actually matters: *can this service be reached at
the address it gave us, on this machine?*

### Networking: `service_locator::resolve_service_address` was removed

**What changed.** The private static helper is gone; its body is inlined into
`on_message_received`.

**How to fix.** Nothing — the member was private and never callable from outside the class. Noted
only because it appears in the diff.

## New APIs

Not breaking, but relevant to the changes above.

| API | Purpose |
| --- | --- |
| `socket_address_variant::is_any()` | True when the address is the wildcard host (`0.0.0.0` / `[::]`). An unspecified address (no family) is **not** any — use `operator bool` to test for that. |
| `socket_address_variant::port(uint16_t)` | Sets the port, keeping the host and family. A no-op on an unspecified address. Needed because `as<T>()` returns a copy, so writes through it are discarded. |
| `json_object_descriptor<T>::is_default(const T&)` | True when every property of the object would be omitted, i.e. it would serialize to `{}`. Answers without building the JSON. |
| `json_converter<converter_t, value_t>` | Concept validating a converter's three static methods. |
