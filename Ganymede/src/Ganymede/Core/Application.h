#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	// Base Ganymede Application. Needs to be implemented by client.
	class GANYMEDE_API Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Run() {};
	};

	// This is an extern function which Ganymede needs you to implement in your client.
	// You are in charge to return an instance of a custom "Application" class you need to implement. 
	// Ganymede will automatically invoke Run() for you. Since Ganymede owns the main function, it can
	// load systems itself, so the client does not have to take care about Ganyemde init stuff.
	Application* CreateApplication();
}