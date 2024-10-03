#pragma once

#include <Ganymede.h>

class WindowCloseEvent;
class WindowInitializeEvent;
class WindowTickEvent;


class GanymedeApp : public Ganymede::Application
{
public:
	GanymedeApp() = default;
	~GanymedeApp() = default;

	void Run() override;

	void GameInit(WindowInitializeEvent&);
	void GameTick(WindowTickEvent& event);
	void GameEnd(WindowCloseEvent&);
};

// This is an external function, invoked by the main function within the Ganymede core.
// That is the main entry point of your Ganymede application.
Ganymede::Application* Ganymede::CreateApplication()
{
	return new GanymedeApp();
}