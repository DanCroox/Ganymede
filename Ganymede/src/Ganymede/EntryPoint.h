#pragma once

#ifdef GM_PLATFORM_WINDOWS

extern Ganymede::Application* Ganymede::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Ganymede::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#endif