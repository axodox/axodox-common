#pragma once
#include "common_includes.h"

namespace Axodox::Graphics
{
	struct AXODOX_COMMON_API Size
	{
		int32_t Width, Height;

		float AspectRatio() const;

		static const Size Zero;

		Size Half() const;
	};
}