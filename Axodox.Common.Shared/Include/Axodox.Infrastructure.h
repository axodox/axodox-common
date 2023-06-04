#pragma once
#include "../includes.h"

#include "Infrastructure/BitwiseOperations.h"
#include "Infrastructure/DependencyContainer.h"
#include "Infrastructure/EventAggregator.h"
#include "Infrastructure/Events.h"
#include "Infrastructure/Traits.h"
#include "Infrastructure/ValuePtr.h"

#ifdef PLATFORM_WINDOWS
#include "Infrastructure/Win32.h"
#include "Infrastructure/Half.h"
#include "Infrastructure/WinRtDependencies.h"
#endif
