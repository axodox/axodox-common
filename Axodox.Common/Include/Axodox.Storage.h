#pragma once
#include "Storage/Stream.h"
#include "Storage/ArrayStream.h"
#include "Storage/MemoryStream.h"
#include "Storage/FileStream.h"
#include "Storage/FileIO.h"

#ifdef PLATFORM_WINDOWS
#include "Storage/UwpStorage.h"
#include "Storage/SettingManager.h"
#include "Storage/ComHelpers.h"
#endif