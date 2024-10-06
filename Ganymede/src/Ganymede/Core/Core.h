#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>

#if defined(_MSC_VER) || defined(__clang__) || defined(__GNUC__)
	#define GM_GENERATE_STATIC_CLASS_ID static int GetStaticClassID() { return __COUNTER__; }
#else
	#error COMPILER_NAME Ganymede only supported on MSVC, Clang and GCC
#endif

// Make sure to implement all platform interfaces in the "Platform" folder for supported platforms
#ifdef GM_PLATFORM_WINDOWS
	#ifdef GM_BUILD_DLL
		#define GANYMEDE_API __declspec(dllexport)
	#else
		#define GANYMEDE_API __declspec(dllimport)
	#endif // GM_BUILD_DLL
#else
	#error Ganymede only supported on Windows!
#endif // GM_PLATFORM_WINDOWS

#define BIT(x) (1 << x)

#ifndef GM_RETAIL
	#define GM_CORE_ASSERTS_ENABLED
#endif // GM_RETAIL

#ifdef GM_CORE_ASSERTS_ENABLED
	#define GM_CORE_ASSERT(condition, message)									\
		{																		\
			if (!(condition))													\
			{																	\
				std::cerr << "Assertion failed: " << #condition << std::endl;	\
				std::cerr << "  file    : " << __FILE__ << std::endl;			\
				std::cerr << "  line    : " << __LINE__	<< std::endl;			\
				std::cerr << "  message : " << message << std::endl;			\
				__debugbreak();													\
			}																	\
		}
#else
	#define GM_CORE_ASSERT(condition, message)
#endif