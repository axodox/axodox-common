#pragma once

//Multiplatform includes
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

//Windows only includes
#ifdef PLATFORM_WINDOWS

//Direct3D
#if defined(USE_DIRECTX) || defined(AXODOX_COMMON_EXPORT)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <d3d11_4.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <memorybuffer.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#endif

//WinRT includes
#include <winrt/base.h>

#ifdef AXODOX_COMMON_EXPORT
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.graphics.imaging.h>
#include <winrt/windows.storage.h>
#include <winrt/windows.storage.streams.h>
#endif

#endif

#pragma warning (disable: 4251)

#ifdef AXODOX_COMMON_EXPORT
#define AXODOX_COMMON_API __declspec(dllexport)
#else
#define AXODOX_COMMON_API __declspec(dllimport)

#ifdef PLATFORM_WINDOWS
#pragma comment (lib,"Holomaps.Common.lib")
#endif
#endif