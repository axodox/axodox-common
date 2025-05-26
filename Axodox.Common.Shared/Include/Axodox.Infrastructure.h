#pragma once
#include "common_includes.h"

#include "Infrastructure/AnyPtr.h"
#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/BufferAllocator.h"
#include "Infrastructure/DependencyContainer.h"
#include "Infrastructure/EventAggregator.h"
#include "Infrastructure/Events.h"
#include "Infrastructure/Traits.h"
#include "Infrastructure/ValuePtr.h"
#include "Infrastructure/Logger.h"
#include "Infrastructure/Text.h"
#include "Infrastructure/Uuid.h"
#include "Infrastructure/Stopwatch.h"
#include "Infrastructure/TypeRegistry.h"
#include "Infrastructure/NamedEnum.h"

#ifdef PLATFORM_WINDOWS
#include "Infrastructure/Win32.h"
#include "Infrastructure/Half.h"
#include "Infrastructure/WinRtDependencies.h"
#include "Infrastructure/Environment.h"
#endif
