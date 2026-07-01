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
			switch (GM_ActiveBackend)
			{
			case GraphicsBackend::OpenGL:
				return std::make_unique<OGLGLFWWindow>(eventSystem);
			case GraphicsBackend::Vulkan:
				return std::make_unique<VKGLFWWindow>(eventSystem);
			default:
				GM_CORE_ASSERT(false, "Not supported");
				return {};
			}
		}
	}
}