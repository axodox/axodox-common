# EventAggregator

`event_aggregator` is a publish/subscribe bus keyed by the C++ type of the event payload. Anyone can publish a value of type `MyEvent` through it; anyone can subscribe a handler `void(MyEvent&)` for the same type. There is no central registration step — the first `raise<T>()` or `subscribe<T>()` call lazily creates the underlying `event_publisher<MyEvent&>` for that type.

This is the right tool when:

- You want decoupled cross-component messaging without each component having to know about the others.
- The "address" of a notification is the event's type, not a specific publisher object.

For tightly-coupled "this object exposes an event" notifications, use a plain `event_owner` + `event_publisher<…>` instead — see [Events](Events.md).

## How it works

Inside the aggregator there is a `std::unordered_map<std::type_index, std::unique_ptr<aggregated_event_base>>`. Each entry wraps an `event_publisher<TEvent&>` whose `event_owner` is the aggregator itself. `raise<TEvent>()` and `subscribe<TEvent>()` look up (or create) the entry by `typeid(TEvent)` and forward to its publisher. A `std::mutex` serialises the lookups so the aggregator is safe to use from multiple threads.

Because the publisher takes its argument by `TEvent&`, handlers can mutate the event in place — useful for "request" events where one subscriber fills in a result field.

## Code examples

### Defining and raising an event

The event itself is just a plain struct. Any type works — the aggregator only uses its `typeid` as a key:

```cpp
#include "Include/Axodox.Infrastructure.h"

struct UserSignedIn
{
  std::string UserId;
  std::chrono::system_clock::time_point At;
};

Axodox::Infrastructure::event_aggregator aggregator;

UserSignedIn payload{ "alice", std::chrono::system_clock::now() };
aggregator.raise(payload);                      // explicit instance

aggregator.raise<UserSignedIn>();               // default-constructed instance
```

The default-construction overload is convenient for marker events that carry no data.

### Subscribing

`subscribe<TEvent>` returns the same `[[nodiscard]] event_subscription` as the regular events system. Hold on to it for as long as the handler should remain registered:

```cpp
class AuditLog
{
  Axodox::Infrastructure::event_subscription _signInSub;

public:
  explicit AuditLog(Axodox::Infrastructure::event_aggregator& bus)
  {
    _signInSub = bus.subscribe<UserSignedIn>(
      [this](UserSignedIn& e) { Record("sign-in", e.UserId); });
  }
};
```

### Mutating an event from a subscriber

Because the handler signature is `void(TEvent&)`, subscribers can write back into the event. This is sometimes used for "ask the bus" patterns where any subscriber can fill in a missing field:

```cpp
struct ResolveSettingsPath
{
  std::filesystem::path Path;        // initially empty; subscribers fill it in
};

aggregator.subscribe<ResolveSettingsPath>([](ResolveSettingsPath& e) {
  if (e.Path.empty()) e.Path = DefaultSettingsLocation();
});

ResolveSettingsPath query;
aggregator.raise(query);
auto path = std::move(query.Path);
```

If multiple subscribers want to contribute, define the convention up front (for example, "first non-empty wins" or "all subscribers append to a vector"). The aggregator does not impose one.

### Sharing the aggregator through DI

The natural home for the bus is the [dependency container](DependencyContainer.md), so any component that needs to talk to the rest of the app can resolve the same instance:

```cpp
Axodox::Infrastructure::dependency_container dependencies;
dependencies.create<Axodox::Infrastructure::event_aggregator>();

// somewhere else
auto bus = dependencies.resolve<Axodox::Infrastructure::event_aggregator>();
bus->raise<UserSignedIn>();
```

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/EventAggregator.h](../../Axodox.Common.Shared/Infrastructure/EventAggregator.h) | The header-only `event_aggregator` class plus the internal `aggregated_event_base` / `aggregated_event<TEvent>` helpers. |
| [Infrastructure/Events.h](../../Axodox.Common.Shared/Infrastructure/Events.h) | Underlying `event_owner` / `event_publisher<…>` / `event_subscription` types. See [Events](Events.md). |
