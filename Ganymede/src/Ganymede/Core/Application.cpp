#include "Application.h"

#include "Ganymede/Events/Event.h"
#include "Ganymede/Platform/Input.h"
#include "Ganymede/Platform/Window.h"#

namespace Ganymede
{
	Application* Application::m_Instance = nullptr;

	Application::Application()
	{
		GM_CORE_ASSERT(m_Instance == nullptr, "It is not allowed to have more than one Application instance at a time.");
		m_Instance = this;

		m_EventSystem = new EventSystem();
		m_InputSystem = InputSystem::Create();
		m_RenderWindow = Window::Create();
	};

	Application::~Application()
	{
		delete m_EventSystem;
		delete m_InputSystem;
		delete m_RenderWindow;
	}

	Application& Application::Get()
	{
		GM_CORE_ASSERT(m_Instance != nullptr, "No Application-instance found. Please create an Application-instance first to use Ganymede.");
		return *m_Instance;
	}

	EventSystem& Application::GetEventSystem()
	{
		return *m_EventSystem;
	}

	InputSystem& Application::GetInputSystem()
	{
		return *m_InputSystem;
	}

	const InputSystem& Application::GetInputSystem() const
	{
		return *m_InputSystem;
	}

	Window& Application::GetRenderWindow()
	{
		return *m_RenderWindow;
	}

	const Window& Application::GetRenderWindow() const
	{
		return *m_RenderWindow;
	}
}