#pragma once

#ifdef GM_PLATFORM_WINDOWS
	#ifdef GM_BUILD_DLL
		#define GANYMEDE_API __declspec(dllexport)
	#else
		#define GANYMEDE_API __declspec(dllimport)
	#endif // GM_BUILD_DLL
#else
	#error Ganymede only supported on Windows!
#endif // GM_PLATFORM_WINDOWS