# LifetimeToken

`lifetime_token` is a single-shot RAII handle that runs an arbitrary `std::function<void()>` when it goes out of scope. APIs that hand out a "temporary registration" / "temporary suppression" / "scoped override" return one of these instead of expecting the caller to call a paired `unregister()` function.

The `[[nodiscard]]` attribute on the type means the compiler will warn when the returned token is dropped on the floor — usually a sign the caller has forgotten that the registration only lasts as long as the token does.

## Anatomy

```cpp
class [[nodiscard]] lifetime_token
{
public:
  lifetime_token() = default;
  lifetime_token(std::function<void()>&& callback);

  lifetime_token(lifetime_token&&) noexcept;
  lifetime_token& operator=(lifetime_token&&) noexcept;

  // Move-only.
  lifetime_token(const lifetime_token&)            = delete;
  lifetime_token& operator=(const lifetime_token&) = delete;

  ~lifetime_token();

  explicit operator bool() const;
  void reset();
};
```

Behaviour:

- The callback fires exactly once — either from `reset()` or from the destructor.
- `reset()` is idempotent: calling it on an already-reset token does nothing.
- Move-assignment runs the destination's callback before taking the source's, so an existing scope is properly closed before the new one starts.
- A default-constructed token holds no callback and is `false` under `operator bool()`.

## Producing a token

The natural pattern is for an API to return a `lifetime_token` whose callback undoes whatever the call did. The Graphics module uses this to scope debug-layer warning suppressions (see `GraphicsDevice::SuppressWarnings`).

```cpp
class WarningGate
{
  std::vector<int> _suppressed;

public:
  Axodox::Infrastructure::lifetime_token Suppress(std::vector<int> ids)
  {
    _suppressed.insert(_suppressed.end(), ids.begin(), ids.end());

    return Axodox::Infrastructure::lifetime_token{
      [this, ids = std::move(ids)]
      {
        // Undo: remove the ids we added.
        for (auto id : ids)
        {
          std::erase(_suppressed, id);
        }
      }
    };
  }
};
```

## Consuming a token

The caller holds the token for as long as the registration should be in effect:

```cpp
WarningGate gate;

{
  auto token = gate.Suppress({ 401, 402 });    // [[nodiscard]] — must be captured
  RunDeviceCalls();                            // warnings are suppressed here
}                                              // token destructs -> warnings restored
```

To end the scope earlier than the surrounding block, call `reset()` or move the token into a temporary that goes out of scope:

```cpp
auto token = gate.Suppress({ 401 });
DoFirstThing();

token.reset();                                  // suppression ends now
DoSecondThing();
```

## When to use it

Reach for `lifetime_token` whenever an API hands out a *paired* registration that must be undone. Compared to alternatives:

- A naked `void Unregister(handle)` is easy to forget; `lifetime_token` makes leaks impossible thanks to RAII and the `[[nodiscard]]` warning.
- A `std::shared_ptr<void>` with a custom deleter works but allocates and is harder to read.
- The library's [`event_subscription`](Events.md) is essentially a specialised `lifetime_token` for the events system; pick this generic version for non-event APIs.

## Files

| File | Contents |
| --- | --- |
| [Infrastructure/LifetimeToken.h](../../Axodox.Common.Shared/Infrastructure/LifetimeToken.h) | The `[[nodiscard]] lifetime_token` class. |
| [Infrastructure/LifetimeToken.cpp](../../Axodox.Common.Shared/Infrastructure/LifetimeToken.cpp) | Move/destructor implementations that fire the callback exactly once. |
