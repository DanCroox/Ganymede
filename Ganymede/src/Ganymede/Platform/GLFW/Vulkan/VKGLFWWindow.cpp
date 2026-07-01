#ifdef GM_PLATFORM_WINDOWS

#include "VKGLFWWindow.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Graphics/Platform/Vulkan/VKContext.h"
#include "Ganymede/Events/Event.h"
#include "Ganymede/Log/Log.h"
#include "Ganymede/Runtime/WindowEvents.h"

#include "GLFW/glfw3.h"

namespace Ganymede
{
	VKGLFWWindow::VKGLFWWindow(EventSystem& eventSystem) :
		Window(eventSystem)
	{}

	VKGLFWWindow::~VKGLFWWindow()
	{
		TerminateWindow();
	}

	bool VKGLFWWindow::Initialize()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_GLFWWindow = glfwCreateWindow(1920, 1080, "Vulkan Window", nullptr, nullptr);
		
		glfwSetWindowUserPointer(m_GLFWWindow, this);
		glfwSetWindowPos(m_GLFWWindow, 200, 100);

		glfwSetInputMode(m_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(m_GLFWWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		VKContext& vkContext = VKContext::GetInstance();
		vkContext.Initialize("Ganymede Core", *m_GLFWWindow);

		GM_CORE_INFO("Vulkan backend successfully initialized");

		return true;
	}

	void* VKGLFWWindow::GetNativeWindow()
	{
		return m_GLFWWindow;
	}

	bool VKGLFWWindow::TryStart()
	{
		m_EventSystem.NotifyEvent(WindowInitializeEvent());

		double gameTime = glfwGetTime();
		double deltaTime = 0;
		unsigned int frameIndex = 0;

		VKContext& vkContext = VKContext::GetInstance();
		while (!glfwWindowShouldClose(m_GLFWWindow))
		{
			glfwPollEvents();

			vkContext.BeginFrame();

			deltaTime = glfwGetTime() - gameTime;
			gameTime = glfwGetTime();
			m_EventSystem.NotifyEvent(WindowTickEvent(deltaTime, gameTime, frameIndex));

			vkContext.EndFrame();

			++frameIndex;
		}

		TerminateWindow();

		m_EventSystem.NotifyEvent(WindowCloseEvent());

		return true;
	}

	void VKGLFWWindow::SetVSyncEnabled(bool isEnabled)
	{
		m_IsVSyncEnabled = isEnabled;
	}

	bool VKGLFWWindow::IsVSyncEnabled() const
	{
		return m_IsVSyncEnabled;
	}

	void VKGLFWWindow::TerminateWindow()
	{
		VKContext::GetInstance().Shutdown();
		GM_CORE_INFO("Vulkan shutdown.");

		glfwDestroyWindow(m_GLFWWindow);
		GM_CORE_INFO("glfw window destroyed.");

		glfwTerminate();
		GM_CORE_INFO("glfw window terminated.");
	}
}
#endif //GM_PLATFORM_WINDOWS