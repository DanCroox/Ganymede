#pragma once

#include <Ganymede.h>

class GanymedeApp : public Ganymede::Application
{
public:
	GanymedeApp() = default;
	~GanymedeApp() = default;

	void Run() override;
};

// ----- Application entry point ------//
Ganymede::Application* Ganymede::CreateApplication()
{
	return new GanymedeApp();
}