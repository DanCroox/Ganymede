#pragma once

#include "Core.h"

namespace Ganymede
{
	class GANYMEDE_API Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Run() {};
	};

	// Application entry point: Needs to be implemented by client
	Application* CreateApplication();
}