#pragma once

#include <cstdio>
#include <functional>
#include <limits>


#define ASSERT(condition, message) if (!(condition)) __debugbreak();
namespace Ganymede
{

	namespace Numbers
	{
		constexpr float MAX_FLOAT = std::numeric_limits<float>::max();
		constexpr float MIN_FLOAT = std::numeric_limits<float>::min();
	}
}