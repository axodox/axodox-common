#pragma once

//Multiplatform includes
#include <cstdint>
#include <variant>
#include <array>
#include <queue>
#include <stack>
#include <ranges>
#include <functional>
#include <future>
#include <shared_mutex>
#include <filesystem>
#include <type_traits>
#include <chrono>
#include <span>
#include <random>
#include <sstream>
#include <set>
#include <typeindex>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <unordered_set>
#include <cstring>
#include <list>
#include <map>
#include <unordered_map>

#if defined(WIN32) && !defined(PLATFORM_WINDOWS)
#define PLATFORM_WINDOWS
#endif

//DirectX math is header-only
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

//Windows only includes
#ifdef PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

//Direct3D
#if defined(USE_DIRECTX) || defined(AXODOX_COMMON_EXPORT)

#include <d3d11_4.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dxgidebug.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <memorybuffer.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "DXGI.lib")
#endif

//WinRT includes
#include <winrt/base.h>
#include <shlwapi.h>
#include <appmodel.h>
#pragma comment (lib, "shlwapi.lib")
#pragma comment(lib, "windowsapp.lib")

#ifdef AXODOX_COMMON_EXPORT
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.foundation.diagnostics.h>
#include <winrt/windows.graphics.imaging.h>
#include <winrt/windows.storage.h>
#include <winrt/windows.storage.streams.h>
#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.composition.h>
#include <winrt/windows.ui.xaml.controls.h>
#include <winrt/windows.system.threading.h>

#include <windows.ui.xaml.media.dxinterop.h>
#include <windows.ui.composition.interop.h>
#endif

#endif

//Linux includes
#ifdef PLATFORM_LINUX
#include <pthread.h>
#endif

#pragma warning (disable: 4251)

#ifdef AXODOX_COMMON_EXPORT
#define AXODOX_COMMON_API __declspec(dllexport)
#else
#define AXODOX_COMMON_API __declspec(dllimport)

#ifdef PLATFORM_WINDOWS
#pragma comment (lib,"Axodox.Common.lib")
#endif
#endif