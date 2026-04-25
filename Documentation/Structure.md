# Repository Structure

`axodox-common` (NuGet id `Axodox.Common`) is a C++20 helper library targeting Windows. It packages reusable building blocks for desktop and Universal Windows Platform (UWP / "Windows Store") C++ projects, and is consumed by other projects either as a NuGet package or as a referenced Visual Studio project.

The build product is a pair of platform-specific DLLs (one for desktop, one for UWP) that share a single set of source files via a Visual Studio shared-items project.

## Top-level layout

```
axodox-common/
├── Axodox.Common.sln           Visual Studio solution
├── Axodox.Common.nuspec        NuGet package manifest
├── Axodox.Common.targets       MSBuild .targets shipped inside the NuGet package
├── build_nuget.ps1             Build + pack script (used locally and by CI)
├── appveyor.yml                AppVeyor CI configuration
├── readme.md / README.md       User-facing documentation
├── LICENSE                     MIT license
├── .gitignore                  Standard Visual Studio gitignore
│
├── Axodox.Common.Shared/       Shared source files (headers + cpp)
├── Axodox.Common.Desktop/      Desktop DLL project (Win32 host)
├── Axodox.Common.Universal/    UWP DLL project (Windows Store host)
│
├── Tools/                      Build helpers (nuget.exe, vswhere.exe)
├── Output/                     NuGet pack output (gitignored)
├── packages/                   NuGet package cache (gitignored)
├── .github/workflows/          GitHub Actions configuration
└── Documentation/              This folder
```

## The three Visual Studio projects

The solution is built from three projects. The shared-items project is the only place real source lives; the two DLL projects only host the build.

| Project | Type | Purpose |
| --- | --- | --- |
| [Axodox.Common.Shared](../Axodox.Common.Shared/) | `.vcxitems` (shared items) | Holds every header and `.cpp`. Imported by the two DLL projects. |
| [Axodox.Common.Desktop](../Axodox.Common.Desktop/) | `.vcxproj` (DynamicLibrary) | Builds the Win32 desktop DLL `Axodox.Common.dll`. |
| [Axodox.Common.Universal](../Axodox.Common.Universal/) | `.vcxproj` (DynamicLibrary) | Builds the UWP / Windows Store DLL `Axodox.Common.dll`. |

Each DLL project is intentionally minimal:

- A single translation unit, [common_includes.cpp](../Axodox.Common.Desktop/common_includes.cpp), is built locally as a precompiled header source (`PrecompiledHeader=Create`).
- All other code is pulled in via the `<ImportGroup Label="Shared">` import of [Axodox.Common.Shared.vcxitems](../Axodox.Common.Shared/Axodox.Common.Shared.vcxitems).
- Both define `AXODOX_COMMON_EXPORT` and `PLATFORM_WINDOWS`. The Universal project additionally defines UWP-specific switches (`WINAPI_FAMILY=WINAPI_FAMILY_APP`, `_WINRT_DLL`, `PLATFORM_UWP`).
- C++ language standard is **stdcpp20**, conformance mode is on, and the precompiled header is `common_includes.h`.

Supported configurations are `Debug` / `Release` × `Win32` / `x64` / `arm64`.

## Inside `Axodox.Common.Shared/`

The shared project is grouped by feature area. Each subfolder is one C++ namespace under `Axodox::`.

```
Axodox.Common.Shared/
├── common_includes.h           Precompiled header — STL / DirectX / WinRT pulls + AXODOX_COMMON_API macro
│
├── Include/                    Public umbrella headers consumers include
│   ├── Axodox.Collections.h
│   ├── Axodox.Graphics.h
│   ├── Axodox.Infrastructure.h
│   ├── Axodox.Json.h
│   ├── Axodox.Storage.h
│   ├── Axodox.Threading.h
│   └── Axodox.UI.h
│
├── Collections/                Axodox::Collections — generic containers and allocators
├── Graphics/                   Axodox::Graphics — Direct3D 11 / Direct2D wrappers (Windows-only)
│   ├── Buffers/                  Constant / Index / Structured / Vertex / generic GraphicsBuffer
│   ├── Devices/                  GraphicsDevice, DeviceContext, DrawingController, base GraphicsResource, GraphicsTypes
│   ├── Math/                     Point, Rect, Size value types
│   ├── Meshes/                   Mesh, IndexedMesh, SimpleMesh, Primitives, VertexDefinitions
│   ├── Shaders/                  ComputeShader / DomainShader / GeometryShader / HullShader / PixelShader / VertexShader
│   ├── States/                   BlendState, DepthStencilState, RasterizerState, SamplerState
│   ├── Swap Chains/              CompositionSwapChain, CoreSwapChain, HwndSwapChain, XamlSwapChain (note: folder name has a space)
│   ├── Textures/                 Texture2D family (RenderTarget2D, DepthStencil2D, StagingTexture2D, etc.) + TextureData
│   └── Helpers.{h,cpp}
├── Infrastructure/             Axodox::Infrastructure — foundational utilities (logging, events, DI, traits, UUID, etc.)
├── Json/                       Axodox::Json — JSON DOM (json_value / json_object / json_array / …) + json_serializer
├── Storage/                    Axodox::Storage — streams, file IO, COM helpers, settings, UWP storage glue
├── Threading/                  Axodox::Threading — events, async ops, blocking collection, dispatchers, parallel, thread pool
└── UI/                         Axodox::UI — minimal Win32 Window wrapper (desktop only)
```

### Module → public umbrella mapping

Consumers are expected to include the umbrella header for the module they need (the README spells this out: `#include "Include/Axodox.*.h"`). Each umbrella file just `#include`s the per-feature headers and is `#ifdef`-guarded for platform-specific subsets.

| Namespace | Umbrella header | Conditional content |
| --- | --- | --- |
| `Axodox::Collections` | [Include/Axodox.Collections.h](../Axodox.Common.Shared/Include/Axodox.Collections.h) | `ObservableExtensions.h` is Windows-only |
| `Axodox::Graphics` | [Include/Axodox.Graphics.h](../Axodox.Common.Shared/Include/Axodox.Graphics.h) | Math types are always available; the rest requires `USE_DIRECTX` and `PLATFORM_WINDOWS` |
| `Axodox::Infrastructure` | [Include/Axodox.Infrastructure.h](../Axodox.Common.Shared/Include/Axodox.Infrastructure.h) | `Win32`, `Half`, `Environment`, `WinRtDependencies` are Windows-only |
| `Axodox::Json` | [Include/Axodox.Json.h](../Axodox.Common.Shared/Include/Axodox.Json.h) | None |
| `Axodox::Storage` | [Include/Axodox.Storage.h](../Axodox.Common.Shared/Include/Axodox.Storage.h) | `UwpStorage`, `SettingManager`, `ComHelpers` are Windows-only |
| `Axodox::Threading` | [Include/Axodox.Threading.h](../Axodox.Common.Shared/Include/Axodox.Threading.h) | `UwpThreading`, `BackgroundThread`, `ThreadPool` are Windows-only |
| `Axodox::UI` | [Include/Axodox.UI.h](../Axodox.Common.Shared/Include/Axodox.UI.h) | Entire content is Windows-only |

### Module summaries

**Infrastructure** — Bedrock utilities that the rest of the library depends on. The tightly-coupled set: `Logger`, `Events`/`EventAggregator` (token-based publish/subscribe), `DependencyContainer` (singleton/transient DI with child containers), `value_ptr` (deep-copying smart pointer used heavily by the Json DOM), `LifetimeToken` / `LifetimeExecutor`, `NamedEnum` (macro-driven reflectable enums), `Traits` / `Concepts`, `Half` (16-bit float), `Uuid`, `Stopwatch`, `BitwiseOperations`, `BufferAllocator`, `TypeRegistry` / `TypeKeySource` (runtime polymorphic type discovery used by JSON), and Win32 / WinRT bridging headers.

**Threading** — Concurrency primitives. `manual_reset_event` and `event_timeout` (in `Threading/Events.h`), `blocking_collection<T>` (the producer/consumer queue used by Logger), `async_operation` / `async_operation_source` (cancellable progress-bearing operations), `manual_dispatcher`, `parallel`, `recursion_lock`, `locked_ptr`, plus the Windows-specific `background_thread`, `thread_pool`, and UWP threading shims.

**Collections** — `aligned_allocator`, `Hasher`, `ObjectPool`, `value_bag` (heterogeneous owning container, see [ValueBag.h](../Axodox.Common.Shared/Collections/ValueBag.h)), and Windows-only `ObservableExtensions` over WinRT collections.

**Storage** — Stream abstraction (`stream` base, `array_stream`, `memory_stream`, `file_stream`), free-function file IO (`read_file`, `try_read_file`, `write_file`, …), `app_folder()` / `lib_folder()` discovery, and Windows-only `SettingManager`, `UwpStorage`, COM helpers.

**Json** — Hand-rolled JSON DOM with `json_value` as a polymorphic base and concrete `json_object`, `json_array`, `json_string`, `json_number`, `json_boolean`, `json_null` derivatives, all owned through `Infrastructure::value_ptr`. `JsonSerializer.h` provides `json_serializer<T>` specializations and a `json_property<T>` mechanism that lets a `json_object_base`-derived class declare members which auto-register their offsets for round-tripping (including support for polymorphic types via `derived_types` and `$type`-discriminated objects).

**Graphics** — Direct3D 11 / Direct2D wrapper layer. `GraphicsDevice` and `GraphicsDeviceContext` are the entry points; resources (`GraphicsResource`, `GraphicsBuffer`, `Texture2D` family) and pipeline state objects (`BlendState`, `DepthStencilState`, `RasterizerState`, `SamplerState`) are built on top. Includes a complete shader stage hierarchy and four swap-chain variants (HWND, XAML, CoreWindow, Composition). Math primitives (`Point`, `Rect`, `Size`) live in this module too and are available even without DirectX.

**UI** — A single thin `Window` class that wraps a desktop `HWND` with `Resized` / `Painting` events. Compiled out of UWP builds.

## Build, packaging, and consumption

### Local build flow

```
build_nuget.ps1
  ├─ vswhere → locate Visual Studio 2022
  ├─ vcvars64.bat → import build env
  ├─ MSBuild.exe Axodox.Common.sln × { Debug, Release } × { x64, x86, arm64 }
  └─ nuget.exe pack Axodox.Common.Patched.nuspec → Output/
```

The nuspec is patched in place during packing to inject `APPVEYOR_BUILD_VERSION`, `APPVEYOR_REPO_BRANCH`, and `APPVEYOR_REPO_COMMIT` when running on AppVeyor.

### Layout of the produced NuGet package

[Axodox.Common.nuspec](../Axodox.Common.nuspec) defines this package layout:

```
build/native/Axodox.Common.targets   ← MSBuild targets imported by consumers
include/                              ← every *.h from Axodox.Common.Shared
lib/desktop/<Platform>/<Config>/      ← Desktop .lib / .exp
lib/universal/<Platform>/<Config>/    ← Universal .lib / .exp
bin/desktop/<Platform>/<Config>/      ← Desktop .dll / .pdb
bin/universal/<Platform>/<Config>/    ← Universal .dll / .pdb
```

### Consumer integration ([Axodox.Common.targets](../Axodox.Common.targets))

The `.targets` file selects desktop vs. universal automatically based on `$(ApplicationType)` (set to `Windows Store` for UWP). It then:

1. Adds the matching `lib/<runtime>/<Platform>/<Configuration>` directory to `<AdditionalLibraryDirectories>` and links every `.lib` it finds.
2. Adds the include directory to `<AdditionalIncludeDirectories>`.
3. Copies the matching DLL/PDB pair to the output directory with `<CopyToOutputDirectory>Always</CopyToOutputDirectory>`.

The environment variable **`AxodoxCommon-Location`** overrides the package-relative paths so a consumer can point at a local source build of either DLL project, and a `<ProjectReference>` to the matching `.vcxproj` is added automatically when this override is set. This is what lets the user develop the library and a consumer side-by-side.

### CI ([appveyor.yml](../appveyor.yml))

Triggers on `main`, builds with `build_nuget.ps1` on the `Visual Studio 2022` AppVeyor image, captures `Output\*.nupkg` as the `NugetPackages` artifact, and deploys it to NuGet via an encrypted API key.

## Cross-cutting build-time switches

These macros gate functionality, mostly inside [common_includes.h](../Axodox.Common.Shared/common_includes.h) and the umbrella headers:

| Macro | Set by | Effect |
| --- | --- | --- |
| `PLATFORM_WINDOWS` | `.vcxproj` (both DLL projects); auto-defined when `WIN32` is set | Enables all Windows-only headers, links Windows SDK libs |
| `PLATFORM_UWP` | Universal `.vcxproj` only | Enables UWP-specific code paths (e.g. logger uses `FileLoggingSession`) |
| `AXODOX_COMMON_EXPORT` | Both DLL projects | Switches `AXODOX_COMMON_API` to `__declspec(dllexport)` and pulls in WinRT projection headers |
| `AXODOX_COMMON_API` | header macro | `dllexport` inside the library, `dllimport` for consumers |
| `USE_DIRECTX` | Consumer-defined | Enables D3D-dependent surface area in `Axodox.Graphics.h`. Implicitly true inside the library because `AXODOX_COMMON_EXPORT` is defined |
| `USE_GLOBAL_DEPENDENCIES` | Consumer-defined | Exposes a process-wide `dependency_container dependencies` from `DependencyContainer.h` |

When consuming the library on Windows, including any header automatically links `Axodox.Common.lib` via `#pragma comment(lib, …)` in `common_includes.h`.
