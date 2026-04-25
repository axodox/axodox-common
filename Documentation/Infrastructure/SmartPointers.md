# Smart Pointers and Buffer Allocation

Three small ownership helpers that fill the gaps between `std::unique_ptr` / `std::shared_ptr` and a hand-rolled allocator.

- **`value_ptr<T>`** — owning pointer with **deep copy**. The library uses it to give the polymorphic JSON DOM value semantics.
- **`any_ptr`** — type-erased owning pointer with runtime-checked `get<T>` / `create<T>`.
- **`buffer_allocator`** — first-fit linear allocator over a fixed virtual range, returning offset/size pairs (`buffer_segment`). Used for slicing up large GPU/staging buffers without per-allocation `new`.

## `value_ptr<T>`

`value_ptr<T>` looks like a `unique_ptr`, but it is copyable. Copying clones the pointee through a stored type-erased copy constructor — captured at the moment of the `value_ptr<T>(new TActual(...))` call — so the polymorphic dynamic type of the original is preserved across copies.

This is what gives the JSON DOM value semantics: a `value_ptr<json_value>` can hold a `json_object`, `json_array`, `json_string`, etc., and copying the outer container deep-copies the entire tree.

### Constructing

```cpp
#include "Include/Axodox.Infrastructure.h"

using namespace Axodox::Infrastructure;

value_ptr<MyType> a;                                   // empty
value_ptr<MyType> b{ new MyType(args) };               // takes ownership
auto c = make_value<MyType>(args…);                    // factory shortcut
value_ptr<MyType> d{ std::make_unique<MyType>(args) }; // adopt unique_ptr

// Polymorphic: store derived through a base value_ptr
value_ptr<Base> e{ new Derived(...) };
```

`make_value<T>(args…)` is the conventional factory — equivalent to `value_ptr<T>(new T(args…))`.

### Copying clones, moving transfers

```cpp
auto original = make_value<MyType>(42);
auto clone    = original;                              // deep copy
auto stolen   = std::move(original);                   // original is now empty
```

If the pointee is not copy-constructible, the copy path throws `std::exception("Cannot copy value!")` at runtime — the constructor is captured as a `std::function<void*(const void*)>` and only the inside path is checked.

### Conversions to standard pointers

```cpp
value_ptr<MyType> p = make_value<MyType>(args);

auto u = std::unique_ptr<MyType>(p);                   // explicit; clones internally
auto s = std::shared_ptr<MyType>(p);                   // explicit; clones internally
```

Both conversions deep-copy — the original `value_ptr` is untouched.

### Equality

`operator==` is enabled only when `supports_equals<T>` is true (see [Traits & Concepts](TypeTraits.md)). Two empty `value_ptr`s compare equal; an empty and a non-empty one compare unequal; otherwise the comparison is delegated to `T::operator==`.

### When to use it

- You need a polymorphic value that is copyable (e.g. variant-like trees, JSON DOM nodes, message envelopes).
- You want clone-on-copy semantics without manual `clone()` virtuals on every base class.

For non-polymorphic, copy-by-value types: prefer the type itself or `std::unique_ptr<T>`. For shared ownership: prefer `std::shared_ptr<T>`. Use `value_ptr` specifically when the combination of *polymorphic + value semantics* matters.

## `any_ptr`

`any_ptr` is a one-cell type-erased owning pointer. It stores a `void*`, a deleter, and the `std::type_index` of the contained type. Operations are runtime-checked: `get<T>()` returns `nullptr` if the stored type is not `T`. Move-only.

### Examples

```cpp
using namespace Axodox::Infrastructure;

any_ptr cell;

auto* a = cell.create<MyType>(args…);                  // construct in place
assert(cell.get<MyType>() == a);
assert(cell.get<OtherType>() == nullptr);              // type mismatch -> nullptr

auto* b = cell.get_or_create<MyType>();                // returns existing if it matches T
cell.reset();                                          // destroy the contents
```

### When to use it

- A class needs to lazily attach an "extension" of an arbitrary type that only some callers know about.
- You want a one-shot type-erased slot without dragging in `std::any` (which copies).

For multi-typed heterogeneous containers, see `Axodox::Collections::value_bag` instead — it is the same idea spread across an unbounded list of cells.

## `buffer_allocator`

`buffer_allocator` is a first-fit linear allocator over a virtual byte range. It does not own memory — it manages offsets. You ask for `size` bytes (with optional `alignment`), and it returns a `buffer_segment{ Start, Size }` carved out of the free list, or an "empty" segment if it could not find a fitting hole.

This is useful for sub-allocating into a single large GPU buffer, a memory-mapped file, or any other monolithic resource where you cannot afford per-allocation operating-system calls.

### Examples

```cpp
using namespace Axodox::Infrastructure;

buffer_allocator allocator{ 64 * 1024 * 1024 };        // 64 MiB virtual range

auto a = allocator.try_allocate(4096, 256);            // 4 KiB, 256-byte aligned
if (!a) return;                                        // operator bool: empty == false

auto b = allocator.try_allocate(8 * 1024);

// later…
allocator.deallocate(a);                                // freed ranges coalesce with neighbours
```

### Behaviour notes

- **Alignment gaps are reclaimed as free space.** If the chosen slot has unused bytes before the aligned start, they are pushed back into the free list.
- **Adjacent free ranges coalesce on `deallocate`.** The allocator scans for free segments touching the freshly-freed range and merges them in place, so the long-term free list does not fragment indefinitely.
- **`try_allocate(size > total)` throws.** Asking for more than the entire virtual size is a logic error rather than a soft failure.
- **`align_memory_offset(offset, alignment)`** is exposed for callers that want to align their own offsets without going through the allocator.
- **Not thread-safe.** Wrap calls in a mutex if multiple threads need to allocate or free concurrently.

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/ValuePtr.h](../../Axodox.Common.Shared/Infrastructure/ValuePtr.h) | `value_ptr<T>` deep-copying owning pointer; `make_value<T>(args…)` factory. |
| [Infrastructure/AnyPtr.h](../../Axodox.Common.Shared/Infrastructure/AnyPtr.h) | `any_ptr` type-erased owning pointer with runtime-checked `get<T>` / `create<T>` / `get_or_create<T>`. |
| [Infrastructure/BufferAllocator.h](../../Axodox.Common.Shared/Infrastructure/BufferAllocator.h) / [.cpp](../../Axodox.Common.Shared/Infrastructure/BufferAllocator.cpp) | `buffer_segment`, `buffer_allocator` (first-fit, coalescing), `align_memory_offset`. |
| [Infrastructure/Traits.h](../../Axodox.Common.Shared/Infrastructure/Traits.h) | `supports_equals` / `supports_not_equals` — used by `value_ptr`'s opt-in equality operators. |
