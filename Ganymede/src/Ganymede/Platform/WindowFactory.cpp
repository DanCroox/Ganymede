#include "WindowFactory.h"

#include "Ganymede/Events/Event.h"
#include "Windows/GLFW/WindowsGLFWWindow.h"
#include "Windows/GLFW/WindowsGLFWInputSystem.h"

namespace Ganymede
{
	namespace WindowFactory
	{
		std::unique_ptr<InputSystem> CreateInputSystem(void* nativeWindow, EventSystem& eventSystem)
		{
			return std::make_unique<WindowsGLFWInputSystem>(nativeWindow, eventSystem);
		}

		std::unique_ptr<Window> CreateApplicationWindow(EventSystem& eventSystem)
		{
			return std::make_unique<WindowsGLFWWindow>(eventSystem);
		}
	}
}