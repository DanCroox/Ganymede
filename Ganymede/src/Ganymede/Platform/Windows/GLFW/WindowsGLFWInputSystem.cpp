#ifdef GM_PLATFORM_WINDOWS

#include "Ganymede/Core/Application.h"
#include "Ganymede/Events/Event.h"
#include "Ganymede/Log/Log.h"
#include "Ganymede/Platform/Window.h"
#include "Ganymede/Runtime/WindowEvents.h"
#include "GLFW/glfw3.h"
#include "WindowsGLFWInputSystem.h"

namespace Ganymede
{
	bool WindowsGLFWInputSystem::Initialize()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(m_NativeWindow);
		// Dont overwrite user pointer elsewhere!
		glfwSetWindowUserPointer(window, this);
		
		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
			WindowResizeEvent event({ width, height });
			Application::Get().GetEventSystem().NotifyEvent(event);
			});

		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
			MouseMoveEvent event({ xpos, ypos });
			Application::Get().GetEventSystem().NotifyEvent(event);
			});

		glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset) {
			MouseScrollEvent event({ xOffset, yOffset });
			Application::Get().GetEventSystem().NotifyEvent(event);
			});

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
				switch (action)
				{
					case GLFW_PRESS:
					{
						const WindowsGLFWInputSystem& input = *static_cast<WindowsGLFWInputSystem*>(glfwGetWindowUserPointer(window));
						MouseButtonPressEvent event(input.ConvertToMouseButtonCode(button));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					case GLFW_RELEASE:
					{
						const WindowsGLFWInputSystem& input = *static_cast<WindowsGLFWInputSystem*>(glfwGetWindowUserPointer(window));
						MouseButtonReleaseEvent event(input.ConvertToMouseButtonCode(button));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					default:
						break;
				}
			});

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				switch (action)
				{
					case GLFW_PRESS:
					{
						const WindowsGLFWInputSystem& input = *static_cast<WindowsGLFWInputSystem*>(glfwGetWindowUserPointer(window));
						KeyPressEvent event(input.ConvertToKeyCode(key));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					case GLFW_RELEASE:
					{
						const WindowsGLFWInputSystem& input = *static_cast<WindowsGLFWInputSystem*>(glfwGetWindowUserPointer(window));
						KeyReleaseEvent event(input.ConvertToKeyCode(key));
						input.m_EventSystem.NotifyEvent(event);
						break;
					}
					default:
						break;
				}
			});

		return true;
	}

	bool WindowsGLFWInputSystem::IsNativeKeyPressed(int nativeKeyCode)
	{
		auto state = glfwGetKey(static_cast<GLFWwindow*>(m_NativeWindow), nativeKeyCode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsGLFWInputSystem::IsNativeMouseButtonPressed(int nativeMouseButtonCode)
	{
		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(m_NativeWindow), nativeMouseButtonCode);
		return state == GLFW_PRESS;
	}

	glm::vec2 WindowsGLFWInputSystem::GetNativeMousePosition()
	{
		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(m_NativeWindow), &x, &y);
		return { x, y };
	}
}

#endif //GM_PLATFORM_WINDOWS