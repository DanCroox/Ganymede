#include "WindowFactory.h"

#include "Ganymede/Events/Event.h"
#include "GLFW/GLFWInputSystem.h"
#include "GLFW/OpenGL/OGLGLFWWindow.h"
#include "GLFW/Vulkan/VKGLFWWindow.h"

namespace Ganymede
{
	namespace WindowFactory
	{
		std::unique_ptr<InputSystem> CreateInputSystem(void* nativeWindow, EventSystem& eventSystem)
		{
			return std::make_unique<GLFWInputSystem>(nativeWindow, eventSystem);
		}

		std::unique_ptr<Window> CreateApplicationWindow(EventSystem& eventSystem)
		{
			return std::make_unique<OGLGLFWWindow>(eventSystem);
		}
	}
}