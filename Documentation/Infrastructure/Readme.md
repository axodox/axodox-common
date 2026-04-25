# Infrastructure

`Axodox::Infrastructure` is the foundational layer of the library. Most other modules depend on it, and almost every consumer touches it through the events system, the logger, the dependency container, or the small smart-pointer / trait helpers.

It is a broad module with many small, mostly independent topics. This page is a high-level map; deep-dives for individual topics live in separate documents alongside this one.

The whole module is reachable via the umbrella header `#include "Include/Axodox.Infrastructure.h"`. Everything lives in the `Axodox::Infrastructure` namespace.

## Topic guides

Each link below is a stand-alone document with the architecture, examples, and file references for that topic.

| Topic | What's inside |
| --- | --- |
| [Events](Events.md) | `event_owner`, `event_publisher<TArgs…>`, `event_handler<TArgs…>`, `[[nodiscard]] event_subscription`, the `no_revoke` opt-out. |
| [EventAggregator](EventAggregator.md) | Type-keyed publish/subscribe bus on top of `event_publisher`. |
| [DependencyContainer](DependencyContainer.md) | `dependency_container`, lifetimes, factory and instance registration, child containers, `dependency_container_ref`. |
| [Logger](Logger.md) | Channel-based asynchronous `logger`, severity threshold, sinks; plus `Stopwatch`. |
| [NamedEnum](NamedEnum.md) | `named_enum` / `named_flags` macros, `named_enum_serializer<T>`, free `to_string<T>` / `parse<T>`. |
| [Type traits, concepts and registry](TypeTraits.md) | `Traits.h`, `Concepts.h`, `TypeKeySource.h`, `TypeRegistry.h`. |
| [Smart pointers](SmartPointers.md) | `value_ptr<T>` deep-copying owning pointer, `any_ptr` type-erased pointer, `buffer_allocator` first-fit. |
| [Bitwise and memory utilities](BitwiseOperations.md) | Typed flag operators, `zero_memory`, `are_equal` family, `to_span` / `to_vector`, `half`. |
| [Uuid](Uuid.md) | 16-byte GUID value type with mixed-endian text round-trip. |
| [Text](Text.md) | `to_lower` (string + wstring) and `split(text, delimiter)`. |
| [LifetimeToken](LifetimeToken.md) | `[[nodiscard]] lifetime_token` RAII callback handle. |
| [Win32 and WinRT bridges](Win32.md) | `Win32.h` (handle traits, `to_wstring`, `make_guid`, …), `Environment.h`, `WinRtDependencies.h`. |

## Files

| File | Topic guide | Summary |
| --- | --- | --- |
| [Include/Axodox.Infrastructure.h](../../Axodox.Common.Shared/Include/Axodox.Infrastructure.h) | (umbrella) | Public umbrella header. Pulls in every per-feature header below; Windows-only ones (`Win32`, `Half`, `Environment`, `WinRtDependencies`) are guarded by `PLATFORM_WINDOWS`. |
| [Infrastructure/Events.h](../../Axodox.Common.Shared/Infrastructure/Events.h) | [Events](Events.md) | `event_owner`, `event_publisher<TArgs…>`, `event_handler<TArgs…>`, `[[nodiscard]] event_subscription`, `no_revoke` opt-out tag. |
| [Infrastructure/EventAggregator.h](../../Axodox.Common.Shared/Infrastructure/EventAggregator.h) | [EventAggregator](EventAggregator.md) | Type-keyed publish/subscribe bus on top of `event_publisher`. |
| [Infrastructure/DependencyContainer.h](../../Axodox.Common.Shared/Infrastructure/DependencyContainer.h) | [DependencyContainer](DependencyContainer.md) | `dependency_container`, `dependency_container_ref`, `dependency_lifetime`, `dependency_pointer_type<T>`, child containers, optional `USE_GLOBAL_DEPENDENCIES` global. |
| [Infrastructure/WinRtDependencies.h](../../Axodox.Common.Shared/Infrastructure/WinRtDependencies.h) | [Win32 and WinRT bridges](Win32.md) | Windows-only specialization of `dependency_pointer_type` for WinRT projection types. |
| [Infrastructure/Logger.h](../../Axodox.Common.Shared/Infrastructure/Logger.h) | [Logger](Logger.md) | `logger` with `std::format`-style overloads, `log_severity` enum, process-wide severity threshold. |
| [Infrastructure/Stopwatch.h](../../Axodox.Common.Shared/Infrastructure/Stopwatch.h) | [Logger](Logger.md) | RAII labelled timer that logs its lifetime on destruction. |
| [Infrastructure/Traits.h](../../Axodox.Common.Shared/Infrastructure/Traits.h) | [Type traits, concepts and registry](TypeTraits.md) | `is_instantiation_of`, `supports_new`, `supports_equals` / `supports_not_equals`, `pointed` / `pointed_t`. |
| [Infrastructure/Concepts.h](../../Axodox.Common.Shared/Infrastructure/Concepts.h) | [Type traits, concepts and registry](TypeTraits.md) | C++20 concepts: `supports_to_string`, `supports_from_string`, `supports_to_from_string`, `trivially_copyable`, `is_pointer_type`, `is_pointing`. |
| [Infrastructure/NamedEnum.h](../../Axodox.Common.Shared/Infrastructure/NamedEnum.h) | [NamedEnum](NamedEnum.md) | `named_enum` / `named_flags` macros, `named_enum_serializer<T>`, free `to_string<T>` / `parse<T>` for enums and integrals. |
| [Infrastructure/TypeKeySource.h](../../Axodox.Common.Shared/Infrastructure/TypeKeySource.h) | [Type traits, concepts and registry](TypeTraits.md) | `type_key_source<T>`, `has_lowercase_type` / `has_uppercase_type` concepts. |
| [Infrastructure/TypeRegistry.h](../../Axodox.Common.Shared/Infrastructure/TypeRegistry.h) | [Type traits, concepts and registry](TypeTraits.md) | `type_registry<TBase>` plus `has_derived_types` concept; backs the JSON polymorphism. |
| [Infrastructure/ValuePtr.h](../../Axodox.Common.Shared/Infrastructure/ValuePtr.h) | [Smart pointers](SmartPointers.md) | `value_ptr<T>` deep-copying owning pointer; `make_value<T>(args…)` factory. |
| [Infrastructure/AnyPtr.h](../../Axodox.Common.Shared/Infrastructure/AnyPtr.h) | [Smart pointers](SmartPointers.md) | `any_ptr` type-erased owning pointer with runtime-checked `get<T>` / `create<T>`. |
| [Infrastructure/BufferAllocator.h](../../Axodox.Common.Shared/Infrastructure/BufferAllocator.h) | [Smart pointers](SmartPointers.md) | `buffer_segment`, `buffer_allocator` (first-fit), `align_memory_offset`. |
| [Infrastructure/BitwiseOperations.h](../../Axodox.Common.Shared/Infrastructure/BitwiseOperations.h) | [Bitwise and memory utilities](BitwiseOperations.md) | Typed flag operators, `zero_memory`, `are_equal` family, `is_default`, `to_span` / `to_vector`. |
| [Infrastructure/Half.h](../../Axodox.Common.Shared/Infrastructure/Half.h) | [Bitwise and memory utilities](BitwiseOperations.md) | Windows-only `half` (IEEE binary16) with implicit `float` conversions. |
| [Infrastructure/Uuid.h](../../Axodox.Common.Shared/Infrastructure/Uuid.h) | [Uuid](Uuid.md) | `uuid` value type with mixed-endian text round-trip. |
| [Infrastructure/Text.h](../../Axodox.Common.Shared/Infrastructure/Text.h) | [Text](Text.md) | `to_lower` (string / wstring) and `split(text, delimiter)`. |
| [Infrastructure/LifetimeToken.h](../../Axodox.Common.Shared/Infrastructure/LifetimeToken.h) | [LifetimeToken](LifetimeToken.md) | `[[nodiscard]] lifetime_token` RAII callback handle. |
| [Infrastructure/Win32.h](../../Axodox.Common.Shared/Infrastructure/Win32.h) | [Win32 and WinRT bridges](Win32.md) | `win32_handle_traits<T>`, `window_handle`, `to_wstring`, `has_package_identity`, `make_guid`. |
| [Infrastructure/Environment.h](../../Axodox.Common.Shared/Infrastructure/Environment.h) | [Win32 and WinRT bridges](Win32.md) | `get_environment_variable(name)` returning UTF-8. |
