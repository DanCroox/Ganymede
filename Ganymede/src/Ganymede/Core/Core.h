#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <typeindex>

typedef size_t ClassID;

#define GM_GENERATE_CLASS_ID(VarName, Class) static const ClassID id = static_cast<ClassID>(std::type_index(typeid(Class)).hash_code());
#define GM_GENERATE_STATIC_CLASS_ID(Class) static ClassID GetStaticClassID() { GM_GENERATE_CLASS_ID(id, Class); return id; }

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