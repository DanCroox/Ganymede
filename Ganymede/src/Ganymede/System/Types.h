#pragma once

#include <cstdio>
#include <functional>
#include <limits>

namespace Ganymede
{
	namespace Numbers
	{
		constexpr float MAX_FLOAT = std::numeric_limits<float>::max();
		constexpr float MIN_FLOAT = std::numeric_limits<float>::min();
		constexpr uint32_t MAX_UINT32 = std::numeric_limits<uint32_t>::max();
		constexpr uint32_t MIN_UINT32 = std::numeric_limits<uint32_t>::min();
	}
}