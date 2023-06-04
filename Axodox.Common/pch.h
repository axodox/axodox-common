#pragma once
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

#pragma warning (disable: 4251)

#ifdef AXODOX_COMMON_EXPORT
#define AXODOX_COMMON_API __declspec(dllexport)
#else
#define AXODOX_COMMON_API __declspec(dllimport)

#ifdef PLATFORM_WINDOWS
#pragma comment (lib,"Holomaps.Common.lib")
#endif
#endif