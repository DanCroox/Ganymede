#pragma once

#include "Ganymede/Core/Core.h"

namespace Ganymede
{
	class EventSystem;
	class InputSystem;
	class Window;

	/// <summary>
	/// Base Ganymede Application. Needs to be implemented by client.
	/// </summary>
	class GANYMEDE_API Application
	{
	public:

		/// <summary>
		/// Ganymede only allows to instanciate one Application object at a time due to the singleton implementation of it.
		/// Ganymede core functions access the static Application instance.
		/// </summary>
		Application();
		virtual ~Application();

		/// <summary>
		/// Main entry-point for the client program. Ganymede calls this during program start inside main function.
		/// </summary>
		virtual void Run() {};

		/// <summary>
		/// Appication instance. An instance of an Application instance can only exist once at a time due to singleton usage across Ganymede.
		/// Ganymede can only work, when there is an instance of Application!
		/// </summary>
		/// <returns>Application instance</returns>
		static Application& Get();

		/// <summary>
		/// Application specific EventSystem for generic events. Use it to subscribe to existing events or create custom events.
		/// </summary>
		/// <returns>EventSystem reference</returns>
		EventSystem& GetEventSystem();
		const EventSystem& GetEventSystem() const;

		/// <summary>
		/// Application specific InputSystem to poll for user inputs (keyboard, mouse etc.)
		/// </summary>
		/// <returns>InputSystem reference</returns>
		InputSystem& GetInputSystem();
		const InputSystem& GetInputSystem() const;

		/// <summary>
		/// Application specific hardware render window. Generates update loop and platform specific input polling.
		/// </summary>
		/// <returns>Window reference</returns>
		Window& GetRenderWindow();
		const Window& GetRenderWindow() const;

	private:
		static Application* m_Instance;

		EventSystem* m_EventSystem = nullptr;
		InputSystem* m_InputSystem = nullptr;
		Window* m_RenderWindow = nullptr;
	};

	/// <summary>
	/// This is an extern function which Ganymede needs you to implement in your client.
	/// You are in charge to return an instance of a custom "Application" class you need to implement. 
	/// Ganymede will automatically invoke Run() for you. Since Ganymede owns the main function, it can
	/// load systems itself, so the client does not have to take care about Ganyemde init stuff.
	/// </summary>
	/// <returns>Your client Application implementation</returns>
	Application* CreateApplication();
}