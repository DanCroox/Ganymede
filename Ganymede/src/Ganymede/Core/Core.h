#pragma once


#if defined(_MSC_VER) || defined(__clang__) || defined(__GNUC__)
	#define GM_GENERATE_STATIC_CLASS_ID static int GetStaticClassID() { return __COUNTER__; }
#else
	#error COMPILER_NAME Ganymede only supported on MSVC, Clang and GCC
#endif

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