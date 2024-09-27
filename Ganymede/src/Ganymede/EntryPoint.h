#pragma once

#ifdef GM_PLATFORM_WINDOWS

#include "Log.h"

extern Ganymede::Application* Ganymede::CreateApplication();

int main(int argc, char** argv)
{
	GM_INIT_LOGGER;
	GM_CORE_INFO("Logger initialized");

	auto app = Ganymede::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#endif // GM_PLATFORM_WINDOWS