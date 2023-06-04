#pragma once
#ifdef PLATFORM_WINDOWS
#include "pch.h"

namespace Axodox::Infrastructure
{
	class AXODOX_COMMON_API half
	{
	public:
		void operator=(float value);
		operator float() const;

	private:
		uint16_t _value;
	};
}
#endif