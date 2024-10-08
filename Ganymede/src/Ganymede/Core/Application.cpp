#include "Application.h"

#include "Ganymede/Events/Event.h"
#include "Ganymede/Log/Log.h"
#include "Ganymede/Platform/Input.h"
#include "Ganymede/Platform/Window.h"#

namespace Ganymede
{
	Application* Application::m_Instance = nullptr;

	Application::Application()
	{
		GM_CORE_ASSERT(m_Instance == nullptr, "It is not allowed to have more than one Application instance at a time.");

		std::unique_ptr<EventSystem> eventSystem = std::make_unique<EventSystem>();
		std::unique_ptr<Window> renderWindow = std::unique_ptr<Window>(Window::Create(*eventSystem));
		if (!renderWindow->Initialize())
		{
			GM_CORE_CRITICAL("Failed to initialize native window.");
			return;
		}

		std::unique_ptr<InputSystem> inputSystem = std::unique_ptr<InputSystem>(InputSystem::Create(renderWindow->GetNativeWindow(), *eventSystem));
		if (!inputSystem->Initialize())
		{
			GM_CORE_CRITICAL("Failed to initialize input system.");
			return;
		}

		m_EventSystem = eventSystem.release();
		m_InputSystem = inputSystem.release();
		m_RenderWindow = renderWindow.release();

		m_Instance = this;
	};

	Application::~Application()
	{
		delete m_RenderWindow;
		delete m_InputSystem;
		delete m_EventSystem;
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

	const EventSystem& Application::GetEventSystem() const
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