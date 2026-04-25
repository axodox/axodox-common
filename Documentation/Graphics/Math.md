# Math

`Graphics::Math` is a tiny set of integer-coordinate value types used by the rest of the Graphics module and by anything that crops, lays out, or describes 2D regions: `Point`, `Size`, and `Rect`. They are all `AXODOX_COMMON_API` plain structs with public fields and a few utility methods.

These three are the only headers in the Graphics module that are available even when `USE_DIRECTX` is not defined — handy when a project just wants the geometry primitives without pulling in D3D.

## Types

### `Point`

```cpp
struct Point
{
  int32_t X, Y;

  static const Point Zero;            // { 0, 0 }
};
```

### `Size`

```cpp
struct Size
{
  int32_t Width, Height;

  float AspectRatio() const;          // Width / Height
  Size  Half() const;                 // { Width/2, Height/2 }

  static const Size Zero;             // { 0, 0 }
};
```

### `Rect`

`Rect` stores edges (`Left, Top, Right, Bottom`) rather than position-and-size, so containment, clamping, and union-style operations are cheap:

```cpp
struct Rect
{
  static const Rect Empty;            // sentinel (max, max, min, min) for accumulation

  int32_t Left, Top, Right, Bottom;

  static Rect FromSize(Size size);                              // (0,0)-(W,H)
  static Rect FromCenterSize(Point center, Size size);
  static Rect FromLeftTopSize(Point leftTop, Size size);

  Rect Extend(Point point) const;     // grow to include point
  Rect Offset(int32_t v) const;       // outset all edges by v
  Rect Fit(float aspectRatio) const;  // recenter, expand to target aspect
  Rect Clamp(Size size) const;        // clamp edges into [0, W/H]
  Rect PushClamp(Size size) const;    // shift before clamping (preserves dims when possible)

  bool   Contains(Rect rect) const;
  Point  LeftTop() const;
  Point  Center() const;
  Size   Size() const;                // {Width, Height}
  int32_t Width()  const;
  int32_t Height() const;

  explicit operator bool() const;     // Left < Right && Top < Bottom
};
```

A few helpers worth noting:

- **`Rect::Empty`** is a *sentinel*, not a zero-size rectangle: `Left = Top = INT32_MAX`, `Right = Bottom = INT32_MIN`. The shape is intentional — the first `Extend(point)` call collapses it to a degenerate single-point rect, so accumulation loops do not need a "first iteration" branch.
- **`Offset(value)`** outsets *all four* edges; pass a negative value to inset.
- **`Fit(aspectRatio)`** keeps the rectangle's center and expands one axis until the target ratio is matched. Use it to compute "letterbox" coordinates around content of a different aspect ratio.
- **`Clamp(size)`** is a hard clip — out-of-bounds edges snap to `0` / `W` / `H`. **`PushClamp(size)`** first translates the rectangle to fit inside the bounds (keeping its width and height when possible), then clamps. Use the latter when sliding a fixed-size selection around an image; use the former for visibility tests.

## Code examples

### Building rectangles

```cpp
#include "Include/Axodox.Graphics.h"

using namespace Axodox::Graphics;

auto canvas = Rect::FromSize({ 1920, 1080 });

auto centered = Rect::FromCenterSize(canvas.Center(), { 256, 256 });
auto offset   = Rect::FromLeftTopSize({ 32, 32 }, { 128, 64 });

if (centered) UseIt(centered);                // operator bool — non-empty
```

### Accumulating a bounding box

```cpp
auto bounds = Rect::Empty;                    // sentinel: max/max/min/min

for (auto p : points) bounds = bounds.Extend(p);

if (!bounds) return;                          // no points -> still degenerate
DrawSelection(bounds);
```

### Letterboxing to a target aspect ratio

```cpp
constexpr float widescreen = 16.f / 9.f;

auto target = canvas.Fit(widescreen);
auto inside = target.Clamp(canvas.Size());     // clip to canvas
```

### Sliding a fixed-size crop

`PushClamp` is the right tool when a UI tool maintains a fixed-size crop window that the user can drag, and the crop must always stay fully inside the image:

```cpp
Rect crop = Rect::FromCenterSize(cursor, { 512, 512 });
crop      = crop.PushClamp(imageSize);         // shifts inwards if needed
```

### Aspect-ratio math on `Size`

```cpp
Size full{ 3840, 2160 };
auto half  = full.Half();                      // { 1920, 1080 }
auto ratio = full.AspectRatio();               // 1.7777…
```

## Files

| File | Contents |
| --- | --- |
| [Graphics/Math/Point.h](../../Axodox.Common.Shared/Graphics/Math/Point.h) / [.cpp](../../Axodox.Common.Shared/Graphics/Math/Point.cpp) | `Point { X, Y }` plus the `Point::Zero` static. |
| [Graphics/Math/Size.h](../../Axodox.Common.Shared/Graphics/Math/Size.h) / [.cpp](../../Axodox.Common.Shared/Graphics/Math/Size.cpp) | `Size { Width, Height }`, `AspectRatio()`, `Half()`, and `Size::Zero`. |
| [Graphics/Math/Rect.h](../../Axodox.Common.Shared/Graphics/Math/Rect.h) / [.cpp](../../Axodox.Common.Shared/Graphics/Math/Rect.cpp) | `Rect { Left, Top, Right, Bottom }`, the static factories, the `Extend` / `Offset` / `Fit` / `Clamp` / `PushClamp` / `Contains` operations, accessors, and the `Rect::Empty` sentinel. |
