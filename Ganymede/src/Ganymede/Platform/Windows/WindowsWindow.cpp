#ifdef GM_PLATFORM_WINDOWS

#include "WindowsWindow.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Ganymede/Log/Log.h"
#include "Ganymede/Runtime/WindowEvents.h"


#include <algorithm>

namespace Ganymede
{
    Window* Window::Create(EventSystem& eventSystem)
    {
        return new WindowsWindow(eventSystem);
    }

    WindowsWindow::~WindowsWindow()
    {
        TerminateWindow();
    }

    bool WindowsWindow::Initialize()
    {
        if (!glfwInit())
        {
            GM_CORE_CRITICAL("glfwInit failed");
            return false;
        }
        GM_CORE_INFO("glfwInit succeeded");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_GLFWWindow = glfwCreateWindow(1920, 1080, "Render View", NULL, NULL);
        if (m_GLFWWindow == nullptr)
        {
            GM_CORE_CRITICAL("glfwCreateWindow failed");
            glfwTerminate();
            return false;
        }
        GM_CORE_INFO("glfwCreateWindow succeeded");

        glfwMakeContextCurrent(m_GLFWWindow);     //Make the window's context current  
        glfwSetWindowUserPointer(m_GLFWWindow, this);
        glfwSetWindowPos(m_GLFWWindow, 200, 100);
        SetVSyncEnabled(false);

        // Init Glew
        if (glewInit() != GLEW_OK)
        {
            GM_CORE_CRITICAL("glewInit failed");
            return false;
        }
        GM_CORE_INFO("glewInit succeeded");

        glfwSetInputMode(m_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(m_GLFWWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        return true;
    }

    void* WindowsWindow::GetNativeWindow()
    {
        return m_GLFWWindow;
    }

    bool WindowsWindow::TryStart()
    {
        m_EventSystem.NotifyEvent(WindowInitializeEvent());

        double gameTime = glfwGetTime();
        double deltaTime = 0;
        unsigned int frameIndex = 0;

        while (!glfwWindowShouldClose(m_GLFWWindow))
        {
            glfwPollEvents();

            deltaTime = glfwGetTime() - gameTime;
            gameTime = glfwGetTime();
            m_EventSystem.NotifyEvent(WindowTickEvent(deltaTime, gameTime, frameIndex));
            ++frameIndex;

            glfwSwapBuffers(m_GLFWWindow);
        }

        TerminateWindow();

        m_EventSystem.NotifyEvent(WindowCloseEvent());

        return true;
    }

    void WindowsWindow::SetVSyncEnabled(bool isEnabled)
    {
        m_IsVSyncEnabled = isEnabled;
        glfwSwapInterval(isEnabled ? 1 : 0);
    }

    bool WindowsWindow::IsVSyncEnabled() const
    {
        return m_IsVSyncEnabled;
    }

    void WindowsWindow::TerminateWindow()
    {
        glfwDestroyWindow(m_GLFWWindow);
        GM_CORE_INFO("glfw window destroyed.");

        glfwTerminate();
        GM_CORE_INFO("glfw window terminated.");
    }
}
#endif //GM_PLATFORM_WINDOWS