# States

Pipeline-state objects in D3D11 are immutable wrappers around a fixed-function configuration. The `States/` subfolder bundles each one (`BlendState`, `DepthStencilState`, `RasterizerState`, `SamplerState`) into a `GraphicsResource`-derived class with a constructor that takes the configuration once and a `Bind(...)` method that sets it on the device context.

State objects are cheap to keep around — create the few you need at startup and reuse them across draw calls.

## What's in here

| Type | Configures |
| --- | --- |
| `BlendState` | The output-merger blend stage. Built from one or more `BlendType` presets, one per render-target slot. |
| `DepthStencilState` | Depth-write enable and depth comparison function. |
| `RasterizerState` | Cull mode, fill mode, depth-bias / slope-scaled depth bias, depth-clip enable. Driven by the `RasterizerFlags` flag enum. |
| `SamplerState` | Texture filtering, addressing mode, border colour. |
| `BlendType` | Preset blend modes: `Opaque`, `Additive`, `Subtractive`, `AlphaBlend`, `Minimum`, `Maximum`. |
| `RasterizerFlags` | Flag enum: `CullNone`, `CullClockwise`, `CullCounterClockwise`, `Wireframe`, `DisableDepthClip`. Combine with the operators from [Infrastructure/BitwiseOperations](../Infrastructure/BitwiseOperations.md). |

## Code examples

### Blending

`BlendState` is constructed either with a single `BlendType` (applied to render-target slot 0) or with a span of them (one per slot, up to `D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT`). The blend factor passed to `Bind` is forwarded to `OMSetBlendState`'s `BlendFactor` argument:

```cpp
#include "Include/Axodox.Graphics.h"

using namespace Axodox::Graphics;

BlendState opaque  { device, BlendType::Opaque };
BlendState alpha   { device, BlendType::AlphaBlend };
BlendState additive{ device, BlendType::Additive };

opaque.Bind();                                          // default factor (1,1,1,1)
alpha.Bind({ 1.f, 1.f, 1.f, 0.5f });                    // pre-multiplied alpha factor
```

For multi-render-target blending — for example, one MRT slot opaque and one additive — pass the per-slot list:

```cpp
std::array slots{ BlendType::Opaque, BlendType::Additive };
BlendState mrt{ device, std::span<const BlendType>{ slots } };
```

### Depth and stencil

`DepthStencilState` is intentionally minimal — it covers the everyday cases of "write depth with `LESS`", "test only", and "always pass":

```cpp
DepthStencilState writeLess { device, /*writeDepth*/ true,  D3D11_COMPARISON_LESS };
DepthStencilState testOnly  { device, /*writeDepth*/ false, D3D11_COMPARISON_LESS_EQUAL };
DepthStencilState noDepth   { device, /*writeDepth*/ false, D3D11_COMPARISON_ALWAYS };

writeLess.Bind();
DrawOpaqueGeometry();

testOnly.Bind();
DrawTransparentOverlay();
```

For full stencil control, drop into the underlying interface — the wrapper does not currently expose stencil parameters.

### Rasterizer

Build a `RasterizerState` from a combination of `RasterizerFlags` plus optional depth-bias values. Combine flags with the typed bitwise helpers from [Infrastructure/BitwiseOperations](../Infrastructure/BitwiseOperations.md):

```cpp
using namespace Axodox::Infrastructure;

RasterizerState defaultState{ device, RasterizerFlags::Default };
RasterizerState backfaceCull{ device, RasterizerFlags::CullClockwise };
RasterizerState wireframe   { device,
  bitwise_or(RasterizerFlags::CullNone, RasterizerFlags::Wireframe) };

RasterizerState shadowMap{
  device,
  RasterizerFlags::CullClockwise,
  /*depthBias*/ 1000,
  /*slopeScaledDepthBias*/ 1.0f
};

shadowMap.Bind();
DrawShadowCasters();
```

Note: the file's enum members include `Default = 0` and `CullNone = 0` (both alias the zero state); pick whichever name reads better at the call site.

### Sampler

`SamplerState` takes the standard D3D filter and address mode plus an optional border colour (used when the address mode is `BORDER`):

```cpp
SamplerState linearClamp { device,
  D3D11_FILTER_MIN_MAG_MIP_LINEAR,
  D3D11_TEXTURE_ADDRESS_CLAMP };

SamplerState pointWrap   { device,
  D3D11_FILTER_MIN_MAG_MIP_POINT,
  D3D11_TEXTURE_ADDRESS_WRAP };

SamplerState linearBorder{ device,
  D3D11_FILTER_MIN_MAG_MIP_LINEAR,
  D3D11_TEXTURE_ADDRESS_BORDER,
  /*borderColor*/ { 0.f, 0.f, 0.f, 1.f } };

linearClamp.Bind(ShaderStage::Pixel, /*slot*/ 0);
```

`Bind` takes the target shader stage and slot, so the same sampler can be installed at different slots in different stages.

### Putting it together

A typical pre-draw block looks like this. Build the states once at startup and reuse:

```cpp
struct PipelineStates
{
  BlendState        Blend;
  DepthStencilState Depth;
  RasterizerState   Rasterizer;
  SamplerState      Sampler;

  PipelineStates(const GraphicsDevice& d) :
    Blend     { d, BlendType::AlphaBlend },
    Depth     { d, /*writeDepth*/ true, D3D11_COMPARISON_LESS },
    Rasterizer{ d, RasterizerFlags::CullClockwise },
    Sampler   { d, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP }
  { }

  void Bind(GraphicsDeviceContext* ctx = nullptr)
  {
    Blend.Bind({ 1.f, 1.f, 1.f, 1.f }, ctx);
    Depth.Bind(ctx);
    Rasterizer.Bind(ctx);
    Sampler.Bind(ShaderStage::Pixel, /*slot*/ 0, ctx);
  }
};
```

## Files

| File | Contents |
| --- | --- |
| [Graphics/States/BlendState.h](../../Axodox.Common.Shared/Graphics/States/BlendState.h) / [.cpp](../../Axodox.Common.Shared/Graphics/States/BlendState.cpp) | `BlendState`, the `BlendType` preset enum, single- and span-based constructors, `Bind(blendFactor, context)`. |
| [Graphics/States/DepthStencilState.h](../../Axodox.Common.Shared/Graphics/States/DepthStencilState.h) / [.cpp](../../Axodox.Common.Shared/Graphics/States/DepthStencilState.cpp) | `DepthStencilState` with the `writeDepth` toggle and the depth-comparison function. |
| [Graphics/States/RasterizerState.h](../../Axodox.Common.Shared/Graphics/States/RasterizerState.h) / [.cpp](../../Axodox.Common.Shared/Graphics/States/RasterizerState.cpp) | `RasterizerState` and the `RasterizerFlags` flag enum (`CullNone` / `CullClockwise` / `CullCounterClockwise` / `Wireframe` / `DisableDepthClip`). |
| [Graphics/States/SamplerState.h](../../Axodox.Common.Shared/Graphics/States/SamplerState.h) / [.cpp](../../Axodox.Common.Shared/Graphics/States/SamplerState.cpp) | `SamplerState` with filter, address mode, optional border colour, and `Bind(stage, slot, context)`. |
